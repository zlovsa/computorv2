#include "PolySolver.hpp"
#include <sstream>
#include "Utils.hpp"

PolySolver::PolySolver():_solved(false){}

PolySolver::PolySolver(int ac, char **av){
	if(!processArguments(ac, av))
		return;
	std::string eq(av[1]);
	init(eq);
}

PolySolver::PolySolver(std::string &eq)
{
	init(eq);
}

void PolySolver::init(std::string &eq){
	if (!readEquation(eq))
		return;
	_solved = true;
	if (!checkDegree())
		return;
	solve();
}

bool PolySolver::error(std::string errmsg){
	_errmsg = errmsg;
	return false;
}

bool PolySolver::processArguments(int ac, char **av){
	if (ac < 2)
		return error(Help());
	int i = 1;
	while(++i<ac){
		std::string pstr(av[i]);
		if(pstr=="-p"){
			i++;
			if(i==ac)
				return error("Incorrect precision provided, exiting.");
			char *end;
			long double p = std::strtold(av[i], &end);
			if (end == av[i] || errno == ERANGE || p < 0 || p==0)
				return error("Incorrect precision provided, exiting.");
			_precision = p;
		}
		if(pstr=="-e")
			_explanation = true;
		if(pstr=="-d"){
			i++;
			if (i == ac)
				return error("Incorrect decimal digits provided, exiting.");
			char *end;
			int d = std::strtol(av[i], &end, 10);
			if (end == av[i] || errno == ERANGE || d <= 0)
				return error("Incorrect decimal digits provided, exiting.");
			_digits = d;
		}
	}
	return true;
}

bool PolySolver::readEquation(std::string &s){
	_expr = NULL;
	try{
		_expr = new Expression(s);
	}catch (const std::exception &e){
		return error(e.what());
	}
	if (!_expr || _expr->getRoot()->opcode != '=')
		return error("Can't read equation.");
	if (_expr->getVars().size() > 1){
		std::stringstream ss;
		ss << "There are " << _expr->getVars().size() << " variables in equation, i can't solve.";
		return error(ss.str());
	}
	_expr->Reduce();
	return true;
}

void collect_polynom(const exprnode *root, std::map<double, Expression::VALUE_TYPE> &polynom, int sign = 1)
{
	char c = root->opcode;
	if (c == '+' || c == '-')
	{
		collect_polynom(root->left, polynom, sign);
		collect_polynom(root->right, polynom, sign * (c == '+' ? 1 : -1));
	}
	else if (c == 'c')
		polynom[0] += sign * root->value.Re();
	else if (c == '^')
		polynom[root->right->value.Re()] += sign;
	else if (c == '*'){
		exprnode *coef = root->left->opcode == 'c' ? root->left : root->right;
		exprnode *pwr = coef == root->right ? root->left : root->right;
		double power = 1;
		if (pwr->opcode != 'v')
			power = pwr->right->value.Re();
		polynom[power] += sign * coef->value.Re();
	}
	else if (c == 'v')
		polynom[1] += sign;
}

bool PolySolver::checkDegree(){
	collect_polynom(_expr->getRoot()->left, polynom);
	int mindeg = 32768;
	deg = 0;
	bool fractional_power = false;
	for (auto i : polynom)
	{
		if (i.second == 0)
			continue;
		if (i.first > deg)
			deg = i.first;
		if (i.first < mindeg)
			mindeg = i.first;
		if (i.first == (double)2)
			a = i.second;
		else if (i.first == (double)1)
			b = i.second;
		else if (i.first == (double)0)
			c = i.second;
		else
			fractional_power = true;
	}
	if (mindeg < 0)
		return error("Negative degree detected, I can't solve.");
	if (deg > 2)
		return error("The polynomial degree is strictly greater than 2, I can't solve.");
	if (fractional_power)
		return error("Fractional power detected, I can't solve.");
	for (int k = 1; k < 3;k++)
		if(polynom[(double)k]==0)
			polynom.erase((double)k);
	return true;
}

std::string PolySolver::getTree() const{
	return _expr->treePrint();
}

bool PolySolver::explane() const {
	return _explanation;
}

Expression::VALUE_TYPE PolySolver::getDiscriminant() const{
	return d;
}

std::string PolySolver::reducedForm() const{
	std::string var = "X";
	if (_expr->getVars().size() > 0)
		var = *(_expr->getVars().begin());
	return printPolynom(polynom, var);
}

bool check_root(Expression::VALUE_TYPE re,
				Expression::VALUE_TYPE im,
				Expression::VALUE_TYPE a,
				Expression::VALUE_TYPE b,
				Expression::VALUE_TYPE c,
				Expression::VALUE_TYPE precision)
{
	return abs(a * re * re - a * im * im + b * re + c) < precision && abs(2 * a * re * im + b * im) < precision;
}

void PolySolver::addroot(Expression::VALUE_TYPE re, Expression::VALUE_TYPE im){
	std::map<double, Expression::VALUE_TYPE> root;
	root[0] = re;
	if (im!=0)
		root[1] = im;
	roots.push_back(root);
}

const std::vector<std::map<double, Expression::VALUE_TYPE>> &PolySolver::getRoots() const
{
	return roots;
}

void PolySolver::solve()
{
	if (deg == 0)
	{
		if (c != 0)
			_errmsg = "There is no solution.";
		else
			_errmsg = "The solution is each real number.";
	}
	else if (deg == 1)
	{
		if (check_root(-c / b, 0, a, b, c, _precision)){
			_errmsg = "The solution is:";
			addroot(-c / b, 0);
		}else
			_errmsg = "There is one solution, but i can't calculate it precise enough.";
	}
	else if (deg == 2)
	{
		d = b * b - 4 * a * c;
		if (d < 0)
		{
			std::stringstream ss;
			ss << "Discriminant is strictly negative, there is no real solutions.";
			Expression::VALUE_TYPE sqrt_d = abs(sqrt(-d) / (2 * a));
			Expression::VALUE_TYPE re = -b / (2 * a);
			if (check_root(re, -sqrt_d, a, b, c, _precision) && check_root(re, sqrt_d, a, b, c, _precision))
			{
				ss << " These are complex ones:";
				addroot(re, -sqrt_d);
				addroot(re, sqrt_d);
			}
			else
				ss << " There are complex ones, but i can't calculate them precise enough";
			_errmsg = ss.str();
		}
		else if (d == 0)
			if (check_root(-b / (2 * a), 0, a, b, c, _precision)){
				_errmsg = "Discriminant is equal to 0, the only real (or two equal) solution is:";
				addroot(-b / (2 * a), 0);
			}else
				_errmsg = "Discriminant is equal to 0, but i can't calculte the only solution precise enough.";
		else{
			Expression::VALUE_TYPE sqrt_d = sqrt(d);
			if (check_root((-b - sqrt_d) / (2 * a), 0, a, b, c, _precision) && check_root((-b + sqrt_d) / (2 * a), 0, a, b, c, _precision)){
				_errmsg = "Discriminant is strictly positive, the two solutions are:";
				addroot((-b - sqrt_d) / (2 * a), 0);
				addroot((-b + sqrt_d) / (2 * a), 0);
			}else
				_errmsg = "Discriminant is strictly positive, there are two solutions, but i can't calculate them precise enough.";
		}
	}
}

bool PolySolver::getSolved() const
{
	return _solved;
}

std::string PolySolver::getMsg() const
{
	return _errmsg;
}

int PolySolver::getDegree() const{
	return deg;
}

PolySolver::PolySolver(const PolySolver &other)
{
	if (this != &other)
		*this = other;
}

PolySolver &PolySolver::operator=(const PolySolver &other)
{
	if (this == &other)
		return (*this);
	_solved = other._solved;
	_errmsg = other._errmsg;
	_explanation = other._explanation;
	roots = other.roots;
	_precision = other._precision;
	deg = other.deg;
	return (*this);
}

PolySolver::~PolySolver(){
	delete _expr;
}

std::string PolySolver::Help()
{
	std::stringstream ss;
	ss << "Computor v1 solves polynomial equations. Usage:" << std::endl
	   << "./computor \"polynom1(x) = polynom2(x)\" [-p precision] [-d digits] [-e]" << std::endl
	   << std::endl
	   << "Polynom is algebraic sum of terms in form a*x^p. For example:" << std::endl
	   << "5 * X^0 + 4 * X^1 - 9.3 * X^2 = 1 * X^0" << std::endl
	   << std::endl
	   << "You may use free form: 5+4x-9.3x^2=1" << std::endl
	   << std::endl
	   << "It is guaranteed that returned solutions meets condition:" << std::endl
	   << "| polynom1(solution) - polynom2(solution) | < precision" << std::endl
	   << std::endl
	   << "-p: precision. Default precision is 1e-6." << std::endl
	   << "-d: decimal digits. Default is 6 decimal digits." << std::endl
	   << "-e: explanation. Shows some explanations." << std::endl;
	return ss.str();
}
