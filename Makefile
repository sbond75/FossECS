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



# https://stackoverflow.com/questions/40621451/makefile-automatically-compile-all-c-files-keeping-o-files-in-separate-folde

CC := /usr/bin/clang++
SRC := src
OBJ := obj

CXXFLAGS=$(NIX_CFLAGS_COMPILE) -std=c++11

SOURCES := $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/Bengine/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS)
	$(CC) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(CXXFLAGS) -I$(SRC) -Ideps/include -c $< -o $@
