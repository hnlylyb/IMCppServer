SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp

IMcpp : $(SOURCES)
	@$(CXX) $(SOURCES) -lpthread -ljsoncpp -o IMcpp

.PHONY : clean
clean :
	@rm IMcpp -f
