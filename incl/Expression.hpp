#pragma once
#include <iostream>
#include <set>
#include <map>
#include "exprnode.hpp"

class Expression
{
public:
	typedef exprnode::VALUE_TYPE VALUE_TYPE;
	class IncorrectExpression : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	Expression();
	Expression(const std::string& s);
	Expression(const Expression &other);
	Expression &operator=(const Expression &other);
	~Expression();
	std::string treePrint() const;
	std::string Print() const;
	const exprnode *getRoot() const;
	const std::set<std::string> getVars() const;
	void Reduce();
	void Evaluate(std::map<std::string, Expression *> &defs);
	void EvaluateRight(std::map<std::string, Expression *> &defs, const std::string &except);

private:
	exprnode *readExpression();
	exprnode *readAddition();
	exprnode *readFactor();
	exprnode *readPower();
	exprnode *readConst();
	exprnode *readMatrix();
	exprnode *readVarFunc();
	bool readDouble(double &val);
	void collectVars();
	void collectVars(exprnode *root);
	void ReduceConstants(exprnode *root);
	void recFindConstants(exprnode *root, const std::string &opcodes, std::set<exprnode *> &consts, double *total, int sign);
	void eval(exprnode *root, std::map<std::string, Expression *> &defs, const std::string &except);

	exprnode *root;
	std::set<std::string> vars;

	char getNextChar();
	char getChar();
	std::string str;
	std::string::size_type i;
};
