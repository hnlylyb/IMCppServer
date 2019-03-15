SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp
JSONCPP = ../jsoncpp

IMcppClient : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -I../Common  -L$(JSONCPP)/lib -lpthread -ljsoncpp -o IMcppClient

.PHONY : clean
clean :
	@rm IMcppClient -f
