#include <iostream>
#include <sstream>
#include "exprnode.hpp"
#include "Utils.hpp"

exprnode::exprnode(exprnode *left, char opcode, exprnode *right) : left(left),
																   right(right),
																   opcode(opcode){}

exprnode::exprnode(exprnode *left, char opcode, exprnode *right,
				   ExprValue value, std::string varname) : left(left),
														   right(right),
														   opcode(opcode),
														   value(value),
														   varname(varname) {}

exprnode::exprnode(ExprValue value) : left(NULL),
									  right(NULL),
									  opcode('c'),
									  value(value) {}

exprnode::exprnode(char opcode, const std::string &varname) : left(NULL),
															  right(NULL),
															  opcode(opcode),
															  varname(varname) {}

bool exprnode::operator==(const ExprValue val)
{
	return (opcode == 'c' && value == val);
}

bool exprnode::operator!=(const ExprValue val){
	return !(*this == val);
}

exprnode::~exprnode(){
	delete left;
	delete right;
}

exprnode *exprnode::clone() const{
	return new exprnode(
		left ? left->clone() : left,
		opcode,
		right ? right->clone() : right,
		value,
		varname);
}

void recprint(const exprnode *root, std::stringstream &ss, char parOpCode, bool right)
{
	(void)parOpCode;
	if (!root)
		return;
	if (root->opcode == 'f'){
		if (root->varname=="abs")
			ss << "|";
		else
			ss << root->varname << "(";
		recprint(root->left, ss, root->opcode, false);
		if (root->varname == "abs")
			ss << "|";
		else
			ss << ")";
	}else if (root->opcode == 'c'){
		bool par = contains("*/-^%m", parOpCode) &&
				   ((root->value.Im() < 0 && fixedout(root->value.Im()) != "0") || fixedout(root->value.Im()) != "0");
		ss << (par ? "(" : "") << root->value << (par ? ")" : "");
	}
	else if (root->opcode == 'v')
		ss << root->varname;
	else {
		bool par = (contains("*/%^m", parOpCode) && contains("+-", root->opcode)) ||
				   (right && parOpCode == '-' && root->opcode == '+');
		ss << (par ? "(" : "");
		recprint(root->left, ss, root->opcode, false);
		if (root->opcode != '^')
			ss << " ";
		ss << root->opcode;
		if (root->opcode != '^')
			ss << " ";
		recprint(root->right, ss, root->opcode, true);
		ss << (par ? ")" : "");
	}
}

std::string exprnode::Print() const{
	std::stringstream ss;
	recprint(this, ss, 0, false);
	return ss.str();
}
