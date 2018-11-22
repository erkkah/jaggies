#!/bin/sh

EXE=test
FRAMEWORKS="-framework Cocoa -framework OpenGL"

clang ${FRAMEWORKS} -o ${EXE} -I ./tigr *.c ./tigr/tigr.c 2>&1 | grep -v "text-based stub"
