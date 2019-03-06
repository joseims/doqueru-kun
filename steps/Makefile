COMPILER = g++
CPPFLAGS = -std=c++11

SRC = $(wildcard *.cpp)
ALL = $(SRC:.cpp=.exe)

all: $(ALL)

%.exe: %.cpp
	$(COMPILER) $(CFLAGS) $< -o $@

clean:
	rm $(ALL)