#pragma once
#include <exception>
#include <string>
#include <vector>

class ExprValue{
public:
	class InvalidOperand : public std::exception{
	public:
		virtual const char *what() const throw(){
			return "Operands mismatch operator";
		}
	};
	ExprValue();
	ExprValue(double re, double im);
	ExprValue(int rows, int cols);
	ExprValue(const ExprValue &other);
	ExprValue &operator=(const ExprValue &other);
	virtual ~ExprValue();
	ExprValue operator+(const ExprValue &rhs);
	ExprValue operator-(const ExprValue &rhs);
	ExprValue operator*(const ExprValue &rhs);
	ExprValue operator/(const ExprValue &rhs);
	ExprValue operator%(const ExprValue &rhs);
	ExprValue operator^(const ExprValue &rhs);
	ExprValue operator&(const ExprValue &rhs);
	bool operator==(const ExprValue &rhs);
	bool operator!=(const ExprValue &rhs);
	virtual std::string toString(bool tree = false) const;
	double &operator()(int row, int col);
	const double &operator()(int row, int col) const;
	double Re() const;
	double Im() const;
	ExprValue Abs() const;
	ExprValue Sqrt() const;
	ExprValue Exp() const;
	ExprValue Ln() const;
	ExprValue Sin() const;
	ExprValue Cos() const;
	ExprValue Tan() const;
	ExprValue Cot() const;
	ExprValue Atan() const;
	ExprValue DegToRad() const;
	ExprValue RadToDeg() const;
	ExprValue Det() const;
	ExprValue Det(int fromrow, int fromcol, int n, int exceptrow, int exceptcol) const;
	ExprValue Cof() const;
	ExprValue Trans() const;
	ExprValue Adj() const;
	ExprValue Inv() const;
	bool isReal() const;
	bool isComplex() const;
	bool isMatrix() const;

private:
	bool scalar;
	double re, im;
	int rows, cols;
	std::vector<double> a;
};

std::ostream &operator<<(std::ostream &stream, const ExprValue &value);
