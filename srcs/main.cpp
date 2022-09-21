#include <iostream>
#include <unistd.h>
#include "Utils.hpp"
#include "MathProcessor.hpp"
#include "ExprValue.hpp"

bool getNextCommand(std::string &cmd){
	if(std::cin && std::cout << "> " && std::getline(std::cin, cmd)){
		if (!isatty(STDIN_FILENO))
			std::cout << cmd << std::endl;
		trim(cmd);
		return cmd != "exit";
	}
	std::cout << "exit" << std::endl;
	return false;
}

int main(){
	std::string cmd;
	MathProcessor mp;
	while (getNextCommand(cmd)){
		std::string answer = mp.processCommand(cmd);
		std::cout << (mp.isError() ? "\033[1;31m" : "\033[1;32m") << answer << "\033[0m";
	}
}
