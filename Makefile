SHELL = /bin/bash
CXX = g++
SOURCES = main.cpp

IMcpp : $(SOURCES)
	@$(CXX) $(SOURCES) -I./jsoncpp/include -lpthread -L./jsoncpp/lib -ljsoncpp -o IMcpp

.PHONY : clean
clean :
	@rm IMcpp -f
