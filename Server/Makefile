SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp
JSONCPP = ../jsoncpp

IMcppServer : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -lpthread -L$(JSONCPP)/lib -ljsoncpp -o IMcppServer

.PHONY : clean
clean :
	@rm IMcpp -f
