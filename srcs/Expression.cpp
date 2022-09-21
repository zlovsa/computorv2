#include "Expression.hpp"
#include <sstream>
#include <iomanip>
#include <map>
#include "Utils.hpp"

const char *Expression::IncorrectExpression::what() const throw()
{
	return "Incorrect expression";
}

Expression::Expression() : root(NULL) {}

Expression::Expression(const std::string &s) : root(NULL)
{
	str = s;
	i = -1;
	root = readExpression();
	if (!root)
		throw IncorrectExpression();
	char c = getChar();
	if (c == '=')
	{
		exprnode *rhs = readExpression();
		if (!rhs)
		{
			delete root;
			root = NULL;
			throw IncorrectExpression();
		}
		exprnode *nexpr = new exprnode(root, c, rhs);
		root = nexpr;
	}
	if (i < str.length())
	{
		delete root;
		root = NULL;
		throw IncorrectExpression();
	}
	collectVars();
}

void Expression::Reduce()
{
	if (root->opcode == '=' && *root->right != ExprValue())
	{
		root->opcode = '-';
		exprnode *nexpr = new exprnode(root, '=', new exprnode(ExprValue()));
		root = nexpr;
	}
}

const std::set<std::string> Expression::getVars() const
{
	return vars;
}

Expression::Expression(const Expression &other)
{
	if (this != &other)
		*this = other;
}

Expression &Expression::operator=(const Expression &other)
{
	if (this == &other)
		return (*this);
	delete root;
	if (other.root)
		root = other.root->clone();
	else
		root = NULL;
	return (*this);
}

Expression::~Expression()
{
	delete root;
}

const exprnode *Expression::getRoot() const
{
	return root;
}

void recTreePrint(exprnode *root, int indent, std::stringstream &ss)
{
	if (!root)
		return;
	std::string str_indent(indent * 2, ' ');
	if (root->opcode == 'c')
		ss << str_indent << root->value.toString(true) << std::endl;
	else if (root->opcode == 'v')
		ss << str_indent << root->varname << std::endl;
	else{
		recTreePrint(root->right, indent + 1, ss);
		ss << str_indent << root->opcode << std::endl;
		recTreePrint(root->left, indent + 1, ss);
	}
}

std::string Expression::treePrint() const
{
	std::stringstream ss;
	recTreePrint(root, 0, ss);
	return ss.str();
}

std::string Expression::Print() const
{
	return root->Print();
}

char Expression::getNextChar()
{
	i++;
	return getChar();
}

char Expression::getChar()
{
	while (i < str.length() && std::isspace(str[i]))
		i++;
	if (i < str.length())
		return str[i];
	return -1;
}

exprnode *del_ret_null(exprnode *ptr1, exprnode *ptr2 = NULL){
	delete ptr1;
	delete ptr2;
	return NULL;
}

exprnode *Expression::readExpression()
{
	exprnode *expr = readAddition();
	if (!expr)
		return NULL;
	char c = getChar();
	while (c == '+' || c == '-'){
		exprnode *rhs = readAddition();
		if (!rhs)
			return del_ret_null(expr);
		exprnode *nexpr = new exprnode(expr, c, rhs);
		c = getChar();
		expr = nexpr;
	}
	return expr;
}

exprnode *Expression::readAddition()
{
	exprnode *expr = readFactor();
	if (!expr)
		return NULL;
	char c = getChar();
	while (c == '*' || c == '/' || c == '%' || std::isalpha(c) || c == '('){
		if (std::isalpha(c) || c == '('){
			i--;
			c = '*';
		}
		if (c == '*' && str.find("**", i) == i){
			getNextChar();
			c = 'm';
		}
		exprnode *rhs = readFactor();
		if (!rhs)
			return del_ret_null(expr);
		exprnode *nexpr = new exprnode(expr, c, rhs);
		c = getChar();
		expr = nexpr;
	}
	return expr;
}

exprnode *Expression::readFactor()
{
	exprnode *expr = readPower();
	if (!expr)
		return NULL;
	char c = getChar();
	if (c == '^'){
		exprnode *rhs = readFactor();
		if (!rhs)
			return del_ret_null(expr);
		exprnode *nexpr = new exprnode(expr, c, rhs);
		c = getChar();
		expr = nexpr;
	}
	return expr;
}

bool Expression::readDouble(double &val)
{
	char *end;
	val = std::strtold(str.c_str() + i, &end);
	if (errno == ERANGE)
		return false;
	std::string s(str.c_str() + i, end - str.c_str() - i);
	size_t dot = s.find_first_of('.');
	int precision = 1000;
	if (dot != std::string::npos)
		precision = s.length() - dot - 1;
	i = end - str.c_str();
	std::string strval = fixedout(val, precision);
	if (s != strval)
		return false;
	return true;
}

exprnode *Expression::readConst()
{
	double val;
	if(readDouble(val))
		return new exprnode(ExprValue(val, 0.));
	return NULL;
}

void Expression::collectVars()
{
	vars.clear();
	collectVars(root);
}

void Expression::collectVars(exprnode *root)
{
	if (!root)
		return;
	if (root->opcode == 'v')
		vars.insert(root->varname);
	collectVars(root->left);
	collectVars(root->right);
}

exprnode *Expression::readMatrix(){
	int row = 0, prev_cols = 0;
	bool read_next_row = true;
	exprnode *expr = new exprnode(ExprValue(1, 1));
	while (read_next_row){
		if (getNextChar() != '[')
			return del_ret_null(expr);
		int col = 0;
		bool read_next_element = true;
		while (read_next_element){
			getNextChar();
			double val;
			if (!readDouble(val))
				return del_ret_null(expr);
			expr->value(row, col++) = val;
			read_next_element = getChar() == ',';
		}
		if (getChar() != ']' || (prev_cols>0 && col!=prev_cols))
			return del_ret_null(expr);
		prev_cols = col;
		read_next_row = getNextChar() == ';';
		row++;
	}
	if (getChar() != ']')
		return del_ret_null(expr);
	getNextChar();
	return expr;
}

exprnode *Expression::readVarFunc(){
	std::string vname;
	char c = getChar();
	while (std::isalpha(c)){
		vname += c;
		c = getNextChar();
	}
	if (vname == "i")
		return new exprnode(ExprValue(0., 1.));
	if (c == '('){
		exprnode *expr = new exprnode('f', lower(vname));
		exprnode *arg = readExpression();
		if (!arg || getChar() != ')')
			return del_ret_null(expr, arg);
		expr->left = arg;
		getNextChar();
		return expr;
	}
	return new exprnode('v', lower(vname));
}

exprnode *Expression::readPower(){
	exprnode *expr = NULL;
	char c = getNextChar();
	if (std::isalpha(c))
		expr = readVarFunc();
	else if (c == '('){
		expr = readExpression();
		c = getChar();
		if (!expr || c != ')')
			return del_ret_null(expr);
		getNextChar();
	}else if(c=='[')
		expr = readMatrix();
	else if(c=='|'){
		exprnode *arg = readExpression();
		c = getChar();
		if (!arg || c != '|')
			return del_ret_null(arg);
		expr = new exprnode('f', "abs");
		expr->left = arg;
		getNextChar();
	}else if (std::isdigit(c) || c == '.')
		expr = readConst();
	else if (c == '-' || c == '+'){
		char c1 = getNextChar();
		i--;
		if (std::isdigit(c1) || c1 == '.')
			expr = readConst();
		else{
			exprnode *nexpr = readFactor();
			if (!nexpr)
				return NULL;
			expr = new exprnode(new exprnode(ExprValue(-1., 0.)), '*', nexpr);
		}
	}
	return expr;
}

exprnode *clone(exprnode *root){
	return root ? root->clone() : root;
}

void clonereplace(exprnode *dest, const exprnode *src){
	if (!dest || !src)
		return;
	exprnode *tmpleft = dest->left;
	exprnode *tmpright = dest->right;
	dest->opcode = src->opcode;
	dest->value = src->value;
	dest->varname = src->varname;
	dest->left = clone(src->left);
	dest->right = clone(src->right);
	delete tmpleft;
	delete tmpright;
}

void recsubst(exprnode *root, std::string substname, exprnode *subst){
	if (!root)
		return;
	recsubst(root->left, substname, subst);
	recsubst(root->right, substname, subst);
	if (root->opcode == 'v' && root->varname == substname)
		clonereplace(root, subst);
}

void Expression::ReduceConstants(exprnode *root){
	if (!root)
		return;
	ExprValue total;
	if (contains("+-", root->opcode)){
		exprnode *tmp = root;
		while (contains("+-", tmp->opcode)){
			if (tmp->right->opcode == 'c' && tmp->right->value.isComplex()){
				if (tmp->opcode == '+')
					total = total + tmp->right->value;
				else
					total = total - tmp->right->value;
				clonereplace(tmp, tmp->left);
			}
			else
				tmp = tmp->left;
		}
		if (tmp->opcode == 'c' && tmp->value.isComplex())
			tmp->value = tmp->value + total;
		else if (total != ExprValue()){
			exprnode *nleft = clone(root);
			delete root->left;
			delete root->right;
			if (total.Re() < 0){
				root->opcode = '-';
				root->right = new exprnode(ExprValue(-total.Re(), -total.Im()));
			}else{
				root->opcode = '+';
				root->right = new exprnode(total);
			}
			root->left = nleft;
		}
	}
}

void setConst(exprnode *root, ExprValue val){
	root->opcode = 'c';
	root->value = val;
	delete root->left;
	root->left = NULL;
	delete root->right;
	root->right = NULL;
}

void Expression::eval(exprnode *root, std::map<std::string, Expression *> &defs, const std::string &except)
{
	if (!root)
		return;
	eval(root->left, defs, except);
	eval(root->right, defs, except);
	if (root->opcode == 'f' && defs.find(lower(root->varname)) != defs.end()){
		exprnode *arg = root->left;
		const exprnode *fun = defs[lower(root->varname)]->getRoot();
		std::string argname = fun->left->left->varname;
		root->left = NULL;
		clonereplace(root, fun->right);
		recsubst(root, argname, arg);
		delete arg;
		eval(root, defs, except);
	}
	if (root->opcode == 'v' && lower(root->varname) != except && defs.find(lower(root->varname)) != defs.end())
		clonereplace(root, defs[lower(root->varname)]->getRoot()->right);
	ReduceConstants(root);
	if (root->opcode == 'f' && lower(root->varname) == "abs" && root->left->opcode == 'c')
		setConst(root, root->left->value.Abs());
	else if (root->opcode == 'f' && lower(root->varname) == "sqrt" && root->left->opcode == 'c')
		setConst(root, root->left->value.Sqrt());
	else if (root->opcode == 'f' && lower(root->varname) == "exp" && root->left->opcode == 'c')
		setConst(root, root->left->value.Exp());
	else if (root->opcode == 'f' && lower(root->varname) == "ln" && root->left->opcode == 'c')
		setConst(root, root->left->value.Ln());
	else if (root->opcode == 'f' && lower(root->varname) == "sin" && root->left->opcode == 'c')
		setConst(root, root->left->value.Sin());
	else if (root->opcode == 'f' && lower(root->varname) == "cos" && root->left->opcode == 'c')
		setConst(root, root->left->value.Cos());
	else if (root->opcode == 'f' && lower(root->varname) == "tan" && root->left->opcode == 'c')
		setConst(root, root->left->value.Tan());
	else if (root->opcode == 'f' && lower(root->varname) == "cot" && root->left->opcode == 'c')
		setConst(root, root->left->value.Cot());
	else if (root->opcode == 'f' && lower(root->varname) == "atan" && root->left->opcode == 'c')
		setConst(root, root->left->value.Atan());
	else if (root->opcode == 'f' && lower(root->varname) == "torad" && root->left->opcode == 'c')
		setConst(root, root->left->value.DegToRad());
	else if (root->opcode == 'f' && lower(root->varname) == "todeg" && root->left->opcode == 'c')
		setConst(root, root->left->value.RadToDeg());
	else if (root->opcode == 'f' && lower(root->varname) == "det" && root->left->opcode == 'c')
		setConst(root, root->left->value.Det());
	else if (root->opcode == 'f' && lower(root->varname) == "cof" && root->left->opcode == 'c')
		setConst(root, root->left->value.Cof());
	else if (root->opcode == 'f' && lower(root->varname) == "trans" && root->left->opcode == 'c')
		setConst(root, root->left->value.Trans());
	else if (root->opcode == 'f' && lower(root->varname) == "adj" && root->left->opcode == 'c')
		setConst(root, root->left->value.Adj());
	else if (root->opcode == 'f' && lower(root->varname) == "inv" && root->left->opcode == 'c')
		setConst(root, root->left->value.Inv());
	else if (root->opcode == 'v' && lower(root->varname) == "pi")
		setConst(root, ExprValue(FT_PI, 0.));
	else if (root->opcode == 'v' && lower(root->varname) == "e")
		setConst(root, ExprValue(FT_E, 0.));
	else if (root->opcode == '+' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value + root->right->value);
	else if (root->opcode == '-' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value - root->right->value);
	else if (root->opcode == '*' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value * root->right->value);
	else if (root->opcode == '/' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value / root->right->value);
	else if (root->opcode == '^' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value ^ root->right->value);
	else if (root->opcode == '%' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value % root->right->value);
	else if (root->opcode == 'm' && root->left->opcode == 'c' && root->right->opcode == 'c')
		setConst(root, root->left->value & root->right->value);
	else if (root->opcode == '*' && root->left->opcode == 'c' && root->left->value == ExprValue())
		setConst(root, ExprValue());
	else if (root->opcode == '*' && root->right->opcode == 'c' && root->right->value == ExprValue())
		setConst(root, ExprValue());
	else if (root->opcode == '+' && root->left->opcode == 'c' && root->left->value == ExprValue())
		clonereplace(root, root->right);
	else if (contains("+-", root->opcode) && root->right->opcode == 'c' && root->right->value == ExprValue())
		clonereplace(root, root->left);
	else if (root->opcode == '*' && root->left->opcode == 'c' && root->left->value == ExprValue(1.,0.))
		clonereplace(root, root->right);
	else if (contains("*/", root->opcode) && root->right->opcode == 'c' && root->right->value == ExprValue(1., 0.))
		clonereplace(root, root->left);
	else if (root->opcode == '^' && root->right->opcode == 'c' && root->right->value == ExprValue(1., 0.))
		clonereplace(root, root->left);
}

void Expression::Evaluate(std::map<std::string, Expression *> &defs)
{
	std::string empty = "";
	eval(root, defs, empty);
}

void Expression::EvaluateRight(std::map<std::string, Expression *> &defs, const std::string &except)
{
	eval(root->right, defs, except);
}
