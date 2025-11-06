#include "AiAgent.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using nlohmann::json;

// --------- utils IO ----------
bool AiAgent::readWholeFile(const std::string& path, std::string& out, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { if (err) *err = "Cannot open file: " + path; return false; }
    std::ostringstream ss; ss << f.rdbuf();
    out = std::move(ss).str();
    return true;
}

// --------- JSON loaders ----------
bool AiAgent::loadConfig(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) return false;
    try {
        auto j = json::parse(s);
        // обязательные поля через at(); port можно оставить как есть, если отсутствует
        cfg_.host   = j.at("host").get<std::string>();
        if (j.contains("port")) cfg_.port = j.at("port").get<std::string>();
        cfg_.api_key = j.at("api_key").get<std::string>();
        return true;
    } catch (const std::exception& e) {
        if (err) *err = std::string("Config parse error: ") + e.what();
        return false;
    }
}

bool AiAgent::loadPrompt(const std::string& path, std::string* err) {
    std::string s;
    if (!readWholeFile(path, s, err)) return false;
    try {
        // Допускаем, что файл — либо строка JSON, либо объект с ключом "prompt"
        json j = json::parse(s);
        if (j.is_string()) {
            prompt_ = j.get<std::string>();
        } else if (j.is_object()) {
            prompt_ = j.at("prompt").get<std::string>();
        } else {
            if (err) *err = "Prompt JSON must be string or object with key 'prompt'";
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        if (err) *err = std::string("Prompt parse error: ") + e.what();
        return false;
    }
}

// ------- Простейший разбор JSON: ожидаем { "text": "<строка>" } -------
std::string AiAgent::extractTextFromJsonBody(const std::string& body) {
    // Если вместе с HTTP-хедерами — отрежем их
    const auto p = body.find("\r\n\r\n");
    const std::string json_part = (p != std::string::npos) ? body.substr(p + 4) : body;

    try {
        auto j = json::parse(json_part);
        return j.at("text").get<std::string>();  // строго ожидаем поле "text"
    } catch (...) {
        return {};
    }
}

// -------- Низкоуровневый HTTPS POST на /api/generate --------
std::optional<std::string> AiAgent::httpsPostGenerate(
        const AiConfig& cfg, const std::string& jsonBody, std::string* err) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) { if (err) *err = "SSL_CTX_new failed"; return std::nullopt; }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { if (err) *err = "socket failed"; SSL_CTX_free(ctx); return std::nullopt; }

    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(cfg.host.c_str(), cfg.port.c_str(), &hints, &res) != 0) {
        if (err) *err = "getaddrinfo failed";
        close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        if (err) *err = "connect failed";
        freeaddrinfo(res); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }
    freeaddrinfo(res);

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) <= 0) {
        if (err) *err = "SSL_connect failed";
        SSL_free(ssl); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    // HTTP запрос
    std::ostringstream req;
    req << "POST /api/generate HTTP/1.1\r\n"
        << "Host: " << cfg.host << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Connection: close\r\n";
    if (!cfg.api_key.empty()) req << "x-api-key: " << cfg.api_key << "\r\n";
    req << "Content-Length: " << jsonBody.size() << "\r\n\r\n"
        << jsonBody;

    const std::string request_str = req.str();
    if (SSL_write(ssl, request_str.c_str(), (int)request_str.size()) <= 0) {
        if (err) *err = "SSL_write failed";
        SSL_free(ssl); close(sock); SSL_CTX_free(ctx); return std::nullopt;
    }

    char buf[4096];
    std::string response;
    int bytes;
    while ((bytes = SSL_read(ssl, buf, sizeof(buf)-1)) > 0) {
        buf[bytes] = '\0';
        response += buf;
    }

    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);

    // ----- Используем nlohmann::json для извлечения "text" -----
    std::string text = extractTextFromJsonBody(response);
    if (text.empty()) {
        if (err) *err = "Cannot extract \"text\" from JSON response";
        return std::nullopt;
    }
    return text;
}

std::optional<std::vector<std::string>> AiAgent::generateMusic(const std::string &mood_description,
                                                                    std::string *outErr)
{
    if (cfg_.host.empty() || cfg_.api_key.empty()) {
        if (outErr) *outErr = "Config not loaded or api_key/host missing";
        return std::nullopt;
    }
    if (prompt_.empty()) {
        if (outErr) *outErr = "Prompt is empty (load it first)";
        return std::nullopt;
    }

    std::string finalprompt = prompt_;
    int pos = finalprompt.find("{mood_description}");
    if (pos != std::string::npos) {
        finalprompt.replace(pos, 18, mood_description);
    }
    json payload = { {"prompt", finalprompt}};
    const std::string body = payload.dump();

    auto AiResponse = httpsPostGenerate(cfg_, body, outErr);
    if (!AiResponse) {
        return std::nullopt;
    }
    try {
        auto j = json::parse(*AiResponse);
        std::vector<std::string> queries;
        for (const auto &item : j) {
            queries.push_back(item.get<std::string>());
        }
        return queries;
    } catch (const std::exception &e) {
        if (outErr) {
            *outErr = std::string("Failed to parse Ai responses: ") + e.what();
        }
        return std::nullopt;
    }
}

std::optional<bool> AiAgent::printResult(const std::vector<TrackInfo> &tracks, const std::string &mood, std::string *outErr)
{
    if (cfg_.host.empty() || cfg_.api_key.empty()) {
        if (outErr) *outErr = "Config not loaded or api_key/host missing";
        return std::nullopt;
    }
    if (tracks.empty()) {
        if (outErr) *outErr = "No tracks to display";
        return std::nullopt;
    }
    if (!mood.empty()) {
        std::cout << "Подобрал для тебя подходящие треки! Генерирую красивый вывод, надо подождать:)\n";
    } else {
        std::cout << "Нашел твой трек!\n";
    }
    
    json tracksJson = json::array();
    for (const auto &track : tracks){
        json trackJson = {
            {"id", track.id},
            {"title", track.title},
            {"artist", track.artist},
            {"album", track.album},
            {"genre", track.genre},
            {"duration", track.duration},
            {"url", track.url}
        };
        tracksJson.push_back(trackJson);
    }

    std::string printprompt = R"(
        Ты - музыкальный ассистент. Тебе нужно ПРОФИЛЬТРОВАТЬ и красиво оформить список треков.
        ВАЖНЫЕ ИНСТРУКЦИИ ПО ФИЛЬТРАЦИИ ДУБЛИКАТОВ:
        1. УДАЛИ ВСЕ ДУБЛИКАТЫ - оставь только УНИКАЛЬНЫЕ треки
        2. Дубликатами считаются:
        - Треки с одинаковым исполнителем И названием (основной критерий)
        - Ремиксы, каверы, караоке-версии оригинальных треков
        - Треки с одинаковым названием, исполнителем, длительностью (±5 секунд) и жанром
        - Live-версии, если есть студийная версия
        3. ПРИОРИТЕТЫ при выборе какой трек оставить:
        - Оригинальные студийные версии
        - Более качественные записи (известные альбомы)
        - Более полные версии (длиннее по времени)

        ПРАВИЛА ВЫВОДА:
        1. Ничего не упоминай про фильтрацию, пользователю нужен только результат, а не то, как он получен
        2. Для каждого трека выводи в формате:
        №. Исполнитель - Название трека
            Альбом: Название альбома
            Жанр: жанр
            Длительность: минуты:секунды
            Ссылка: url
        {{MOOD_EXPLANATION}}
        4. Используй эмодзи для оформления

        ПРЕДУПРЕЖДЕНИЕ: Если ты выведешь дубликаты, пользователь будет расстроен!
        Список всех найденных треков (включая дубликаты для фильтрации):
        {{TRACKS_DATA}}
        
        Отфильтруй и выведи только уникальные треки:
    )";

    std::string jsonTracksStr = tracksJson.dump();
    size_t pos = printprompt.find("{{TRACKS_DATA}}");
    if (pos != std::string::npos) {
        printprompt.replace(pos, 15, jsonTracksStr);
    }
    std::string moodExp;
    if (!mood.empty()) {
        moodExp = "3. В конце объясни, почему эти конкретные треки подходят для настроения: \"" +
        mood + "\". При этом само настроение в кавычках не повторяй";
    } else {
        moodExp = "3. Не объясняй, почему треки подходят, просто выведи их по инструкции";
    }
    pos = printprompt.find("{{MOOD_EXPLANATION}}");
    if (pos != std::string::npos) {
            printprompt.replace(pos, 20, moodExp);
    }
    json payload = {{"prompt", printprompt}};
    const std::string body = payload.dump();
    auto AiResponse = httpsPostGenerate(cfg_, body, outErr);
    if (!AiResponse) {
        std::cout << "AI request failed\n";
        return false;
    }
    if (AiResponse->empty()) {
        std::cout << "AI returned empty response\n";
        return false;
    }
    std::cout << "\n" << *AiResponse << "\n";
    std::cout << "Приятного прослушивания!\n";
    return true;
}