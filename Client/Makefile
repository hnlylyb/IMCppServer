SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp ../Common/Thread.cpp
JSONCPP = ../jsoncpp

IMcppClient : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -I../Common -lpthread -L$(JSONCPP)/lib -ljsoncpp -o IMcppClient

.PHONY : clean
clean :
	@rm IMcppClient -f
