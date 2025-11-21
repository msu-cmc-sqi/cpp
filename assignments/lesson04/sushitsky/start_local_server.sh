#!/bin/bash
cd llama.cpp
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
cd ..
./llama.cpp/build/bin/llama-server   -m llama.cpp/models/tinyllama-1.1b-chat-v1.0.Q2_K.gguf   -c 4096 -ngl 999   --host 127.0.0.1 --port 8080