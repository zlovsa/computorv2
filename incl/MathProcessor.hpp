#pragma once
#include <iostream>
#include <set>
#include <map>
#include "Expression.hpp"
#include "exprnode.hpp"

class MathProcessor
{
public:
	MathProcessor();
	MathProcessor(const MathProcessor &other);
	MathProcessor &operator=(const MathProcessor &other);
	~MathProcessor();
	std::string processCommand(std::string &command);
	bool isError() const;

private:
	bool error = false;
	std::map<std::string, Expression *> defs;
	std::set<std::string> built_in_funcs{"abs", "sqrt", "exp", "ln", "sin",
										 "cos", "tan", "cot", "atan", "torad",
										 "todeg", "det", "cof", "trans", "inv",
										 "adj"};
	std::set<std::string> built_in_vars{"pi", "e"};
};
