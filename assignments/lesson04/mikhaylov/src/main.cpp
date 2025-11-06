#include "CodeReviewer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    // Usage: ./app [config.json] [prompt.json] [source_file]
    const std::string configPath = (argc > 1) ? argv[1] : "config.json";
    const std::string promptPath  = (argc > 2) ? argv[2] : "prompt.json";
    const std::string sourcePath  = (argc > 3) ? argv[3] : "source.txt"; // default source.txt

    CodeReviewer reviewer;
    std::string err;
    if (!reviewer.init(configPath, promptPath, &err)) {
        std::cerr << "Init error: " << err << "\n";
        return 1;
    }

    // Read source file
    std::ifstream f(sourcePath);
    if (!f) {
        std::cerr << "Cannot open source file: " << sourcePath << "\n";
        return 2;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    reviewer.setSourceCode(ss.str());

    auto resp = reviewer.run(&err);
    if (!resp) {
        std::cerr << "Review failed: " << err << "\n";
        return 3;
    }

    std::cout << *resp << "\n";
    return 0;
}
