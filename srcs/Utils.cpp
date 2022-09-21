#include "Utils.hpp"
#include <iomanip>

const char *DomainError::what() const throw()
{
	return "Domain error";
}

std::string &lower(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(),
				   [](unsigned char c)
				   { return std::tolower(c); });
	return s;
}

void trim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
									{ return !std::isspace(ch); }));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
						 { return !std::isspace(ch); })
				.base(),
			s.end());
}

double sqrt(double d)
{
	if (d < 0)
		throw DomainError();
	else if (d == 0)
		return 0;
	double r0 = 0, r1 = 1;
	while (r0 != r1){
		r0 = r1;
		r1 = (r0 + d / r0) / 2;
	}
	return r1;
}

double abs(double x){
	return x < 0 ? -x : x;
}

double exp(double x){
	double r = 1;
	double delta = 1;
	for (int n = 1; r + delta != r; n++){
		delta *= x / n;
		r += delta;
	}
	return r;
}

double ln(double x){
	if(0<x && x<=1){
		x -= 1;
		double r = x;
		double xpwr = x;
		double delta = 1;
		for (int n = 2; r + delta != r; n++){
			xpwr *= -x;
			delta = xpwr / n;
			r += delta;
		}
		return r;
	}else if(x>1)
		return -ln(1 / x);
	throw DomainError();
}

double reduce_period(double x, double t){
	long long n = (long long)(x / t);
	x -= n * t;
	if(x<0)
		x += t;
	return x;
}

double sin(double x){
	if (x < 0 || x >= 2 * FT_PI)
		return sin(reduce_period(x, 2 * FT_PI));
	if (x >= FT_PI)
		return -sin(x - FT_PI);
	if (x >= FT_PI / 2)
		return cos(x - FT_PI / 2);
	if (x > FT_PI / 4)
		return cos(FT_PI / 2 - x);
	double r = x;
	double delta = x;
	for (int n = 2; r + delta != r; n += 2){
		delta *= -x * x / (n * n + n);
		r += delta;
	}
	return r;
}

double cos(double x){
	if (x < 0 || x >= 2 * FT_PI)
		return cos(reduce_period(x, 2 * FT_PI));
	if (x >= FT_PI)
		return -cos(x - FT_PI);
	if (x >= FT_PI / 2)
		return -sin(x - FT_PI / 2);
	if (x > FT_PI / 4)
		return sin(FT_PI / 2 - x);
	double r = 1;
	double delta = 1;
	for (int n = 1; r + delta != r; n += 2){
		delta *= -x * x / (n * n + n);
		r += delta;
	}
	return r;
}

double tan(double x){
	double cosine = cos(x);
	if (cosine == 0)
		throw DomainError();
	return sin(x) / cosine;
}

double cot(double x){
	double sine = sin(x);
	if (sine == 0)
		throw DomainError();
	return cos(x) / sine;
}

double atan(double x){
	if (x < 0)
		return -atan(-x);
	if (x > 1)
		return FT_PI / 2 - atan(1 / x);
	if (x > 1 - 1e-6)
		return FT_PI / 4;
	double r = x;
	double xpwr = x;
	double delta = 1;
	for (int n = 3; r + delta != r; n += 2){
		xpwr *= -x * x;
		delta = xpwr / n;
		r += delta;
	}
	return r;
}

double atan2(double y, double x){
	if(x==0 && y==0)
		throw DomainError();
	if(x==0 && y>0)
		return FT_PI / 2;
	if(x==0 && y<0)
		return -FT_PI / 2;
	if(x>0)
		return atan(y / x);
	if(y>=0)
		return atan(y / x) + FT_PI;
	return atan(y / x) - FT_PI;
}

double degtorad(double x){
	return x * FT_PI / 180.;
}

double radtodeg(double x){
	return x * 180. / FT_PI;
}

double pow(double x, double y){
	if(x==0 || x==1)
		return x;
	if (y == 0)
		return 1;
	if(y==(long long)y){
		double r = 1;
		for (long long p = (long long)abs(y); p > 0; p >>= 1){
			if(p&1)
				r *= x;
			x *= x;
		}
		return y < 0 ? 1 / r : r;
	}
	if (x > 0)
		return exp(y * ln(x));
	throw DomainError();
}

bool contains(const std::string &where, char what){
	return where.find(what) != std::string::npos;
}

int min(int a, int b){
	return a < b ? a : b;
}

int max(int a, int b){
	return a > b ? a : b;
}

std::string fixedout(double val, int precision){
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(precision) << (val == 0 ? 0 : val);
	std::string s = ss.str();
	size_t dot = s.find_first_of('.'), last = s.find_last_not_of(".0");
	if (dot != std::string::npos)
		return s.substr(0, dot > last + 1 ? dot : (last + 1));
	else
		return s;
}

bool poly_zero(std::map<double, double> polynom){
	for (auto i : polynom)
		if (i.second != 0)
			return false;
	return true;
}

std::string printPolynom(std::map<double, double> polynom, std::string varname){
	bool printplus = false;
	std::stringstream ss;
	while(polynom.size()>1 && polynom.rbegin()->second==0)
		polynom.erase(polynom.rbegin()->first);
	for (auto i : polynom){
		if (i.second == 0 && !poly_zero(polynom))
			continue;
		double k = abs(i.second);
		if (!printplus){
			if (i.second < 0 && fixedout(i.second) != "0")
				ss << "-";
		}else{
			if (i.second < 0 && fixedout(i.second) != "0")
				ss << " - ";
			else
				ss << " + ";
		}
		if (i.first == 0 || fixedout(k) != "1")
			ss << fixedout(k);
		if (i.first != 0){
			ss << varname;
			if (i.first < 0)
				ss << "^(" << i.first << ")";
			else if (i.first > 1)
				ss << "^" << i.first;
		}
		printplus = true;
	}
	return ss.str();
}

double det(std::vector<std::vector<double>> a){
	size_t n = a.size();
	if(!n)
		return 1.;
	if (a[0].size() != n)
		throw DomainError();
	double det = 1;
	for (size_t i = 0; i < n; ++i){
		size_t k = i;
		for (size_t j = i + 1; j < n; ++j)
			if (abs(a[j][i]) > abs(a[k][i]))
				k = j;
		if (abs(a[k][i]) < 1e-9)
			return 0;
		std::swap(a[i], a[k]);
		det *= (i == k ? 1 : -1) * a[i][i];
		for (size_t j = i + 1; j < n; ++j)
			a[i][j] /= a[i][i];
		for (size_t j = i+1; j < n; ++j)
			if (abs(a[j][i]) > 1e-9)
				for (size_t k = i + 1; k < n; ++k)
					a[j][k] -= a[i][k] * a[j][i];
	}
	return det;
}
