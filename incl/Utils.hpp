#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>
#define FT_PI 3.14159265358979323846
#define FT_E 2.71828182845904523536

class DomainError : public std::exception
{
public:
	virtual const char *what() const throw();
};

std::string &lower(std::string &s);
void trim(std::string &s);
double sqrt(double d);
double abs(double x);
double exp(double x);
double ln(double x);
double sin(double x);
double cos(double x);
double tan(double x);
double cot(double x);
double atan(double x);
double atan2(double y, double x);
double degtorad(double x);
double radtodeg(double x);
double pow(double x, double y);
bool contains(const std::string &where, char what);
int min(int a, int b);
int max(int a, int b);
std::string printPolynom(std::map<double, double> polynom, std::string varname);
std::string fixedout(double val, int precision = 6);
double det(std::vector<std::vector<double>> a);