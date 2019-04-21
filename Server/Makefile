SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp Server.cpp BasicController.cpp IMController.cpp
JSONCPP = ../jsoncpp

IMcppServer : $(SOURCES)
	@$(CXX) $(SOURCES) -I$(JSONCPP)/include -I../Common -L$(JSONCPP)/lib -lpthread -ljsoncpp -o IMcppServer

.PHONY : clean
clean :
	@rm IMcppServer -f
