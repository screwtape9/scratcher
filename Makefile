CC=g++
CXXFLAGS=-std=c++11 -c -Wall -Wextra -Wshadow -Wredundant-decls -Wunreachable-code -Winline
INCLUDES=
LIBS=-lcurl

ifeq ($(DEBUG),1)
CXXFLAGS+=-g
endif

OBJ:=game.o cget.o main.o
EXE=demo

COMPILE.1=$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $<
ifeq ($(VERBOSE),)
COMPILE=@printf "  > compiling %s\n" $(<F) && $(COMPILE.1)
else
COMPILE=$(COMPILE.1)
endif

%.o: %.cpp
	$(COMPILE)

.PHONY: all clean rebuild

all: $(EXE)

$(EXE): $(OBJ) $(OUTPUT_DIR)
	$(CC) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f $(EXE) $(OBJ)

rebuild: clean all
