/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Tiffany Thani
 */

/*
	calculator08buggy.cpp | From: Programming -- Principles and Practice Using C++, by Bjarne Stroustrup
	We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/

#include "std_lib_facilities.h"
#include <math.h>

struct Token {
	char kind;
	double value;
	string name;
	Token():kind(0), value(0) {}
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { }
	Token(char ch, string n) : kind(ch), name(n) {}  //error Fix1: added Token constructor that accepts strings
};

class Token_stream {
	public:
	int fill=0;
	Token buffer[2];
public:
	Token_stream() :fill(0) { buffer[0] = Token(0); buffer[1] = Token(0); }

	Token get();
	void unget(Token t) { if ( fill == 0 || fill == 1)  buffer[fill++] = t ; }

	void ignore(char);
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
	if (fill == 1 || fill == 2) { /*cout << "her cuz fill is" << fill << endl;*/ return buffer[--fill]; }
	//cout << " in get, didn't return cuz fill is 0" << endl;
	char ch;
	cin >> ch;
	//cout << " got the char.. " << endl;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();
	double val;
	cin >> val;
	return Token(number, val);
	}
	default:  // means we are dealing with a word (either a var or a quit/let directive )
		//cout << " we are heeeeere" << std::endl;
		if (isalpha(ch) || ch =='_') {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;  //error fix5: change = to += (runtime error)
			cin.unget();
			if (s == "let" || s == "l" || 
				s == "LET" || s == "L") return Token(let);     // error fix2:  added options for s to equal
			if (s == "quit" || s =="q" ||
				 s == "QUIT" || s == "Q") return Token(quit);  // Error Fix3:  added more ptions for s to equal
																// Error Fix4: passed quit instead of name 
			//cout << " s is: *" << s << "* " << endl;
			return Token(name, s); 
		}
		error("Bad tokeeen");
	}
}

void Token_stream::ignore(char c)
{
	if (fill > 0 && c == buffer[fill].kind) {
		fill--;
		return;
	}
	fill--;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
	bool isConst;
	Variable(string n, double v, bool cantChange) :name(n), value(v), isConst(cantChange) { }
};

vector<Variable> names;

double get_value(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(string s, double d)
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

Token_stream ts;

double expression();

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	
		double d = expression();
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
	}
	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
		return get_value(t.name);
	default:
		error("primaryyyy expected");
	}
}

double term()
{
	//cout << " getting left rn" << endl;
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
		case '%':  // error fix6: added modulo functionality 
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left = fmod(left, d);
		break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression()
{
	//cout << "in expr, getting left for term " << endl;
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		case '_':

		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration(bool isConst)
{
	Token t = ts.get();
	if (t.kind != 'a') error("name expected in declaration");
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression();
	names.push_back(Variable(name, d, isConst));
	return d;
}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		//cout << " found let" << endl;
		return declaration(false);
	default:
		if (t.name == "const"){
			return declaration(true);
		}
		if (is_declared(t.name) ){
			Token t2 = ts.get();
			if (t2.kind == '='){
				double d = expression();
				for (int i = 0; i < names.size(); i++){
				//for (Variable v : names){
					if (names[i].name == t.name){
						if (names[i].isConst == false){
							names[i].value = d;
							return d;
						}
						else {
							cout << "CAN'T CHANGE A CONST VALUE!!" << endl;
							return -1;
						}
					}
				}
				return 0;
			}
			else {
				ts.unget(t2);
			}
		}
		
		
		//cout << " found *" << t.kind << "*" << endl;
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		//cout <<" aftr first get, fill is " << ts.fill << " and t.kind is " << t.kind << endl;
		while (t.kind == print) t = ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()

try {
	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}
