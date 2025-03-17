#!/usr/bin/env bash
x86_64-w64-mingw32-gcc src/main.c -o bin/main -I./include -L./lib -l:wlibraylib.a -lm -lwinmm -lgdi32
