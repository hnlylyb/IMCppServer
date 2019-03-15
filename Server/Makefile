SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp ../Common/Thread.cpp
JSONCPP = ../jsoncpp

IMcppServer : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -I../Common -lpthread -L$(JSONCPP)/lib -ljsoncpp -o IMcppServer

.PHONY : clean
clean :
	@rm IMcppServer -f
