#pragma once
#include "Expression.hpp"
#include <string>
#include <vector>
#include <map>

class PolySolver
{
public:
	PolySolver();
	PolySolver(int ac, char **av);
	PolySolver(std::string &eq);
	PolySolver(const PolySolver &other);
	void init(std::string &eq);
	PolySolver &operator=(const PolySolver &other);
	~PolySolver();
	static std::string Help();
	bool getSolved() const;
	std::string getMsg() const;
	std::string reducedForm() const;
	int getDegree() const;
	const std::vector<std::map<double, double>> &getRoots() const;
	std::string getTree() const;
	bool explane() const;
	double getDiscriminant() const;

private:
	bool processArguments(int ac, char **av);
	bool readEquation(std::string &s);
	bool checkDegree();
	bool error(std::string errmsg);
	void solve();
	void addroot(double re, double im);

	Expression *_expr = NULL;
	bool _solved = false;
	bool _explanation = false;
	std::string _errmsg;
	long double _precision = 1e-6l;
	std::map<double, double> polynom;
	std::vector<std::map<double, double>> roots;
	int deg;
	int _digits = 6;
	double a = 0, b = 0, c = 0, d = 0;
};
