#include "ExprValue.hpp"
#include <sstream>
#include "Utils.hpp"

ExprValue ExprValue::operator+ (const ExprValue &rhs){
	if(scalar && rhs.scalar)
		return ExprValue(re + rhs.re, im + rhs.im);
	if (!scalar && !rhs.scalar && rows == rhs.rows && cols == rhs.cols){
		ExprValue m(rows, cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < cols; col++)
				m(row, col) = (*this)(row, col) + rhs(row, col);
		return m;
	}
	throw InvalidOperand();
}

ExprValue ExprValue::operator- (const ExprValue &rhs){
	if(scalar && rhs.scalar)
		return ExprValue(re - rhs.re, im - rhs.im);
	if (!scalar && !rhs.scalar && rows == rhs.rows && cols == rhs.cols)
	{
		ExprValue m(rows, cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < cols; col++)
				m(row, col) = (*this)(row, col) - rhs(row, col);
		return m;
	}
	throw InvalidOperand();
}

ExprValue ExprValue::operator* (const ExprValue &rhs){
	if(scalar && rhs.scalar)
		return ExprValue(re * rhs.re - im * rhs.im, im * rhs.re + re * rhs.im);
	if (!scalar && !rhs.scalar && rows == rhs.rows && cols == rhs.cols){
		ExprValue m(rows, cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < cols; col++)
				m(row, col) = (*this)(row, col) * rhs(row, col);
		return m;
	}
	if(scalar && im==0 && !rhs.scalar){
		ExprValue m(rhs.rows, rhs.cols);
		for (int row = 0; row < rhs.rows; row++)
			for (int col = 0; col < rhs.cols; col++)
				m(row, col) = re * rhs(row, col);
		return m;
	}
	if(!scalar && rhs.scalar && rhs.im==0){
		ExprValue m(rows, cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < cols; col++)
				m(row, col) = (*this)(row, col) * rhs.re;
		return m;
	}
	throw InvalidOperand();
}

ExprValue ExprValue::operator/ (const ExprValue &rhs){
	if(scalar && rhs.scalar){
		double c2d2 = rhs.re * rhs.re + rhs.im * rhs.im;
		if (c2d2 == 0)
			throw DomainError();
		return ExprValue((re * rhs.re + im * rhs.im) / c2d2,
						 (im * rhs.re - re * rhs.im) / c2d2);
	}
	if(!scalar && rhs.scalar && rhs.im==0){
		ExprValue m(rows, cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < cols; col++)
				m(row, col) = (*this)(row, col) / rhs.re;
		return m;
	}
	throw InvalidOperand();
}

ExprValue ExprValue::operator% (const ExprValue &rhs){
	if (scalar && im == 0 && re == (int)re 
		&& rhs.scalar && rhs.im == 0 && rhs.re == (int)rhs.re){
		return ExprValue((double)((int)re % (int)rhs.re), 0.);
	}
	throw InvalidOperand();
}

ExprValue ExprValue::operator^ (const ExprValue &rhs){
	// if (scalar && im == 0 && rhs.scalar && rhs.im == 0)
	// 	return ExprValue(pow(re, rhs.re), 0.);
	if (scalar && rhs.scalar && rhs.im == 0 && rhs.re==(int)rhs.re){
		ExprValue r(1., 0.);
		ExprValue x = *this;
		for (long long p = (long long)abs(rhs.re); p > 0; p >>= 1){
			if (p & 1)
				r = r * x;
			x = x * x;
		}
		return rhs.re < 0 ? ExprValue(1., 0.) / r : r;
	}
	if (scalar && rhs.scalar && rhs.im == 0){
		double rn = pow(re * re + im * im, rhs.re / 2);
		double nphi = atan2(im, re) * rhs.re;
		return ExprValue(rn * cos(nphi), rn * sin(nphi));
	}
	if (!scalar && rows == cols && rhs.scalar && rhs.im == 0 && rhs.re == (int)rhs.re && rhs.re >= 0){
		ExprValue r(rows, cols);
		for (int i = 0; i < rows;i++)
			r(i, i) = 1.;
		ExprValue x = *this;
		for (long long p = rhs.re; p > 0; p >>= 1){
			if (p & 1)
				r = r & x;
			x = x & x;
		}
		return r;
	}
	if (!scalar && rows == cols && rhs.scalar && rhs.im == 0 && rhs.re == (int)rhs.re)
		return Inv() ^ ExprValue(-rhs.re, 0.);
	throw InvalidOperand();
}

bool ExprValue::operator==(const ExprValue &rhs){
	return (scalar && rhs.scalar && re == rhs.re && im == rhs.im) 
		|| (!scalar && !rhs.scalar && a == rhs.a);
}

bool ExprValue::operator!=(const ExprValue &rhs){
	return !(*this == rhs);
}

ExprValue ExprValue::operator&(const ExprValue &rhs){
	if(!scalar && !rhs.scalar && cols==rhs.rows){
		ExprValue m(rows, rhs.cols);
		for (int row = 0; row < rows; row++)
			for (int col = 0; col < rhs.cols; col++){
				m(row, col) = 0;
				for (int k = 0; k < cols;k++)
					m(row, col) += (*this)(row, k) * rhs(k, col);
			}
		return m;
	}
	throw InvalidOperand();
}

double &ExprValue::operator()(int row, int col){
	if (!scalar && 0 <= row && 0 <= col){
		if (col >= cols)
			cols = col + 1;
		if (row >= rows)
			rows = row + 1;
		a.resize(rows * cols, 0.);
		return a[row * cols + col];
	}
	throw InvalidOperand();
}

const double &ExprValue::operator()(int row, int col) const{
	if (!scalar && 0 <= row && row < rows && 0 <= col && col < cols)
		return a[row * cols + col];
	throw InvalidOperand();
}

ExprValue::ExprValue() : scalar(true), re(0), im(0) {}

ExprValue::ExprValue(double re, double im) : scalar(true), re(re), im(im) {}

ExprValue::ExprValue(int rows, int cols) : scalar(false), rows(rows), cols(cols){
	if (rows < 1 || cols < 1)
		throw InvalidOperand();
	a.resize(rows * cols, 0.);
}

ExprValue::ExprValue(const ExprValue &other){
	if (this != &other)
		*this = other;
}

ExprValue &ExprValue::operator=(const ExprValue &other){
	if (this == &other)
		return (*this);
	this->a = other.a;
	this->im = other.im;
	this->rows = other.rows;
	this->cols = other.cols;
	this->re = other.re;
	this->scalar = other.scalar;
	return (*this);
}

double ExprValue::Re() const{
	return re;
}

double ExprValue::Im() const{
	return im;
}

bool ExprValue::isReal() const{
	return scalar && im == 0;
}

bool ExprValue::isComplex() const{
	return scalar;
}

bool ExprValue::isMatrix() const{
	return !scalar;
}

ExprValue ExprValue::Abs() const{
	if (!scalar)
		return Det();
	return ExprValue(sqrt(re * re + im * im), 0.);
}

ExprValue ExprValue::Sqrt() const{
	if(!scalar || im!=0)
		throw DomainError();
	return ExprValue(sqrt(re), 0.);
}

ExprValue ExprValue::Exp() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(exp(re), 0.);
}

ExprValue ExprValue::Ln() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(ln(re), 0.);
}

ExprValue ExprValue::Sin() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(sin(re), 0.);
}

ExprValue ExprValue::Cos() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(cos(re), 0.);
}

ExprValue ExprValue::Tan() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(tan(re), 0.);
}

ExprValue ExprValue::Cot() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(cot(re), 0.);
}

ExprValue ExprValue::Atan() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(atan(re), 0.);
}

ExprValue ExprValue::DegToRad() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(degtorad(re), 0.);
}

ExprValue ExprValue::RadToDeg() const{
	if (!scalar || im != 0)
		throw DomainError();
	return ExprValue(radtodeg(re), 0.);
}

ExprValue ExprValue::Det() const{
	if(scalar || rows!=cols)
		throw InvalidOperand();
	return Det(0, 0, rows, -1, -1);
}

ExprValue ExprValue::Det(int fromrow, int fromcol, int size, int exceptrow, int exceptcol) const{
	int n = exceptrow > -1 ? size - 1 : size;
	std::vector<std::vector<double>> a(n, std::vector<double>(n));
	int row = 0;
	for (int i = fromrow; i < fromrow + size; i++){
		if(i==exceptrow)
			continue;
		int col = 0;
		for (int j = fromcol; j < fromcol + size;j++){
			if(j==exceptcol)
				continue;
			a[row][col++] = (*this)(i, j);
		}
		row++;
	}
	return ExprValue(det(a), 0.);
}

ExprValue ExprValue::Cof()const{
	if (scalar || rows != cols)
		throw InvalidOperand();
	ExprValue r(rows, cols);
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < cols;col++)
			r(row, col) = Det(0, 0, rows, row, col).Re() * ((row + col) % 2 ? -1 : 1);
	return r;
}

ExprValue ExprValue::Trans() const{
	if (scalar)
		throw InvalidOperand();
	ExprValue r(cols, rows);
	for (int row = 0; row < rows; row++)
		for (int col = 0; col < cols; col++)
			r(col, row) = (*this)(row, col);
	return r;
}

ExprValue ExprValue::Adj() const{
	return Cof().Trans();
}

ExprValue ExprValue::Inv() const{
	ExprValue det = Det();
	if (abs(det.Re()) < 1e-9)
		throw InvalidOperand();
	return Adj() / det;
}

ExprValue::~ExprValue() {}

std::string ExprValue::toString(bool tree) const{
	std::stringstream ss;
	if(scalar){
		std::map<double, double> c;
		c[0] = re;
		c[1] = im;
		ss << printPolynom(c, "i");		
	}else{
		if(tree)
			ss << "[";
		for (int row = 0; row < rows; row++){
			ss << "[ ";
			for (int col = 0; col < cols; col++)
				ss << a[row * cols + col] << (col < cols - 1 ? " , " : "");
			ss << " ]";
			if (row < rows - 1)
				ss << (tree ? ";" : "\n  ");
		}
		if (tree)
			ss << "]";
	}
	return ss.str();
}

std::ostream &operator<<(std::ostream &stream, const ExprValue &value){
	stream << value.toString();
	return stream;
}
