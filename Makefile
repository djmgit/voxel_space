COMPILER = cc
SOURCE_LIBS_OSX = -Iinclude/
OPT = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
OUT = -o voxel
CFILES = main.c
PLATFORM := $(shell uname)

ifeq ($(PLATFORM), Darwin)
	COMPILER = clang
	SOURCE_LIBS_OSX = -Ilib/darwin -Iinclude/
	OPT = -Llib/darwin/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/darwin/libraylib.a
endif

build:
	$(COMPILER) $(CFILES) $(SOURCE_LIBS_OSX) $(OUT) $(OPT)

run:
	./voxel

clean:
	rm -rf ./voxel
