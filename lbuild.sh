#!/usr/bin/env bash
gcc src/main.c -o bin/main -I./include -L./lib -l:llibraylib.a -lm
