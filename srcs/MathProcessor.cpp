#include "MathProcessor.hpp"
#include "PolySolver.hpp"
#include "Utils.hpp"

MathProcessor::MathProcessor(){}

MathProcessor::MathProcessor(const MathProcessor &other){
	if (this != &other)
		*this = other;
}

MathProcessor &MathProcessor::operator=(const MathProcessor &other)
{
	if (this == &other)
		return (*this);
	defs.clear();
	for(auto i:other.defs)
		defs[i.first] = new Expression(*i.second);
	return (*this);
}

MathProcessor::~MathProcessor(){
	for (auto i : defs)
		delete i.second;
}

bool MathProcessor::isError() const{
	return error;
}

std::string MathProcessor::processCommand(std::string &command){
	error = false;
	if (command.empty() || command.front() == '#')
		return "";
	std::stringstream ss;
	if (command == "ls"){
		for (auto i : defs)
			ss << "  " << i.first << " : " << i.second->Print() << std::endl;
		ss << "  " << defs.size() << " defines total." << std::endl;
		return ss.str();
	}
	enum QueryType{
		define,
		solve,
		calculate
	};
	QueryType qType = define;
	if (!command.empty() && command.back() == '?'){
		qType = solve;
		command.pop_back();
		trim(command);
		if (!command.empty() && command.back() == '='){
			qType = calculate;
			command.pop_back();
		}
	}
	ss << "  ";
	Expression *_expr = NULL;
	try{
		_expr = new Expression(command);
	}catch (const std::exception &e){
		delete _expr;
		ss << "Can't read query!" << std::endl;
		error = true;
		return ss.str();
	}
	if (!_expr || (qType != calculate && _expr->getRoot()->opcode != '=') 
		|| (qType == define && _expr->getRoot()->left->opcode != 'v' && _expr->getRoot()->left->opcode != 'f')
		|| (qType == define && _expr->getRoot()->left->opcode == 'f' && _expr->getRoot()->left->left->opcode != 'v')
		|| (qType == define && _expr->getRoot()->left->opcode == 'v' && built_in_vars.find(lower(_expr->getRoot()->left->varname))!=built_in_vars.end())
		|| (qType == define && _expr->getRoot()->left->opcode == 'f' && built_in_funcs.find(lower(_expr->getRoot()->left->varname))!=built_in_funcs.end())){
		delete _expr;
		ss << "Incorrect query!" << std::endl;
		error = true;
		return ss.str();
	}
	//std::cout << _expr->treePrint() << std::endl;
	try{
		if (qType != define)
			_expr->Evaluate(defs);
		else{
			std::string argname = "";
			if (_expr->getRoot()->left->opcode == 'f')
				argname = lower(_expr->getRoot()->left->left->varname);
			_expr->EvaluateRight(defs, argname);
		}
	}catch(const std::exception &e){
		ss << e.what() << std::endl;
		delete _expr;
		error = true;
		return ss.str();
	}
	if (qType == define){
		ss << _expr->getRoot()->right->Print() << std::endl;
		//std::cout << _expr->treePrint();
		std::string key = _expr->getRoot()->left->varname;
		if (defs.find(key) != defs.end())
			delete defs[key];
		defs[key] = _expr;
	}
	if (qType == calculate){
		ss << _expr->Print() << std::endl;
		delete _expr;
	}
	if (qType == solve){
		std::string eq = _expr->Print();
		ss << eq << std::endl;
		PolySolver solver(eq);
		ss << "  " << solver.getMsg() << std::endl;
		for (auto x : solver.getRoots())
			ss << "  " << printPolynom(x, "i") << std::endl;
		delete _expr;
	}
	return ss.str();
}
