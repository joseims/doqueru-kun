COMPILER=g++
CPPFLAGS=-std=c++11

HELLO=hello_worldo
CONTAINER_CPP=doqueru.cpp
CONTAINER_EXE=doqueru-kun.exe
ALL_EXECUTABLES := $(shell ls | grep .exe)

NEWROOT=doquerinhos_shell
STEPS=steps

.PHONY: all clean $(STEPS)

all: $(HELLO) $(CONTAINER_EXE) $(STEPS)

$(HELLO): $(HELLO).cpp
	$(COMPILER) $(CPPFLAGS) $(HELLO).cpp -o $(HELLO).exe
	$(COMPILER) $(CPPFLAGS) $(HELLO).cpp -o $(NEWROOT)/$(HELLO).exe

$(CONTAINER_EXE): $(CONTAINER_CPP)
	$(COMPILER) $(CPPFLAGS) $(CONTAINER_CPP) -o $(CONTAINER_EXE)
    
$(STEPS):
	cd $(STEPS) && $(MAKE)

clean:
	rm $(ALL_EXECUTABLES) $(NEWROOT)/$(HELLO).exe
	cd $(STEPS) && $(MAKE) clean

