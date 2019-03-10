SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp Thread.cpp
JSONCPP = ../jsoncpp

IMcppServer : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -lpthread -L$(JSONCPP)/lib -ljsoncpp -o IMcppServer

.PHONY : clean
clean :
	@rm IMcppServer -f
