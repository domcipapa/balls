#!/usr/bin/env bash
# https://github.com/BallerIndustries/raylib-wasm-example

emcc -o ./out/index.html \
    src/main.c -Os -Wall lib/wasmlibraylib.a \
    -I. -I include \
    -L. -L lib \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    --shell-file ./shell.html \
    -s TOTAL_STACK=64MB \
    -s INITIAL_MEMORY=128MB \
    -s ASSERTIONS \
    -DPLATFORM_WEB

    #    --preload-file resources \
