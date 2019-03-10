SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp
JSONCPP = ../jsoncpp

IMcppClient : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -L$(JSONCPP)/lib -ljsoncpp -o IMcppClient

.PHONY : clean
clean :
	@rm IMcpp -f
