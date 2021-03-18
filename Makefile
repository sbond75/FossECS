# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

# SRCS = *.cpp Bengine/*.cpp

# DEPDIR = .deps

# MAKEDEPEND = cc -E

# CXXFLAGS =-I .

# %.o : %.cpp $(DEPDIR)/%.d
# 	@$(MAKEDEPEND)
# 	$(COMPILE.cpp) -o $@ $<

# DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
# $(DEPFILES):
# include $(wildcard $(DEPFILES))





# Detect OS: https://stackoverflow.com/questions/714100/os-detecting-makefile #
ifeq ($(OS),Windows_NT)
    #CCFLAGS += -D WIN32
    HostOS := Windows
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        #CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            #CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            #CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        HostOS := Linux
        #CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        HostOS := macOS
        #CCFLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        #CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        #CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        #CCFLAGS += -D ARM
    endif
endif
# #


# https://stackoverflow.com/questions/40621451/makefile-automatically-compile-all-c-files-keeping-o-files-in-separate-folde

#CC := /usr/bin/clang++
CC := c++
SRC := src
OBJ := obj

# Also add SDL2/ to the include path to fix SDL2_mixer having `:3:
# /nix/store/19fv4b3cgrpni0mggwqvh214afmaafs9-SDL2_mixer-2.0.4/include/SDL2/SDL_mixer.h:25:10: fatal error: 'SDL_stdinc.h'
#        file not found
# #include "SDL_stdinc.h"` we use this:
# pkg-config is useful, based on https://github.com/NixOS/nixpkgs/issues/32079#issuecomment-347020691 : for example:
# `echo $(pkg-config --cflags SDL2)` gives:
# `-D_THREAD_SAFE -I/nix/store/793akkzljrkwqldaqh6k0kp642q0z4lq-SDL2-2.0.12-dev/include/SDL2`
# and ` echo $(pkg-config --cflags-only-I SDL2)` gives:
# `-I/nix/store/793akkzljrkwqldaqh6k0kp642q0z4lq-SDL2-2.0.12-dev/include/SDL2`
ADDITIONAL_SDL_INCLUDES=`pkg-config --cflags-only-I SDL2`

CXXFLAGS=$(NIX_CFLAGS_COMPILE) $(ADDITIONAL_SDL_INCLUDES) -std=c++14 -g3 -O0
LDFLAGS=$(NIX_LDFLAGS)

SOURCES := $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/Bengine/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

ifeq ($(HostOS),Linux)
    LINUX_LIBS := -lGL
endif
ifeq ($(HostOS),macOS)
    MACOS_LIBS := -framework OpenGL
endif
LIBS := $(LINUX_LIBS) $(MACOS_LIBS) -lSDL2 -lSDL2_ttf -lGLEW -lSDL2_mixer -stdlib=libc++ -lc++fs

export MACOSX_DEPLOYMENT_TARGET = 10.15

EXECUTABLE_RESULT=ecs

all: $(EXECUTABLE_RESULT)

$(EXECUTABLE_RESULT): $(OBJECTS)
	@echo Detected OS: $(HostOS)
	 # @echo $(MACOS_LIBS)
	$(CC) $^ -o $@ $(LIBS) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(CXXFLAGS) -I$(SRC) -isystemdeps/include -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE_RESULT)
