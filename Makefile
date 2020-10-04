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

SRC := src
OBJ := obj

SOURCES := $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/Bengine/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(LDFLAGS) -lgl -lSDL2 -lSDL2_ttf -lglew $(CXXFLAGS) -I$(SRC) -c $< -o $@
