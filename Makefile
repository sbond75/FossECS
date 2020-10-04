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

CC := /usr/bin/clang++
SRC := src
OBJ := obj

CXXFLAGS=$(NIX_CFLAGS_COMPILE) -std=c++14
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

all: fossECS

fossECS: $(OBJECTS)
	@echo Detected OS: $(HostOS)
	 # @echo $(MACOS_LIBS)
	$(CC) $^ -o $@ $(LIBS) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(CXXFLAGS) -I$(SRC) -Ideps/include -c $< -o $@

clean:
	rm -f $(OBJECTS)
