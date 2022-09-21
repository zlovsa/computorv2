#pragma once
#include <string>
#include <vector>
#include "ExprValue.hpp"

struct exprnode
{
	typedef double VALUE_TYPE;
	exprnode *left;
	exprnode *right;

	char opcode;
	ExprValue value;
	std::string varname;
	exprnode(exprnode *left, char opcode, exprnode *right);
	exprnode(exprnode *left, char opcode, exprnode *right,
			 ExprValue value, std::string varname);
	exprnode(const ExprValue value);
	exprnode(const char opcode, const std::string &varname);
	~exprnode();
	exprnode *clone() const;
	std::string Print() const;

	bool operator==(const ExprValue val);
	bool operator!=(const ExprValue val);
};
