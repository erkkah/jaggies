CFLAGS = -I.. -I./tigr -DGL_SILENCE_DEPRECATION
LDFLAGS = -lm
ifeq ($(OS),Windows_NT)
	LDFLAGS += -ld3d9
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
	else ifeq ($(UNAME_S),Linux)
		LDFLAGS += -lGLU -lGL -lX11
	endif
endif

all: test

tigr/tigr.c:
	git submodule update --init

test : test.c ./tigr/tigr.c ../jaggies.c
	gcc $^ -g -o $@ $(CFLAGS) $(LDFLAGS)
