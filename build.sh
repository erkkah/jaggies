#!/bin/sh

EXE=test
FRAMEWORKS="-framework Cocoa -framework OpenGL"

clang ${FRAMEWORKS} -g -o ${EXE} -I ./tigr *.c ./tigr/tigr.c 2>&1 | grep -v "text-based stub"
