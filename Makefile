CXX = g++

CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -O3 -I include -MMD

LDFLAGS = -lncurses

EXEC = vm

SOURCES = $(wildcard *.cc) main.cc

OBJECTS = $(SOURCES:.cc=.o)

DEPENDS = $(OBJECTS:.o=.d)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

.PHONY: clean run format

clean:
	rm -f $(OBJECTS) $(EXEC) $(DEPENDS)


