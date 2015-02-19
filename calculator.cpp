// calculator.cpp

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

//-----------------------------------------------------------------------------

std::istringstream is;
const char number = '8';
const char print = ';';

//-----------------------------------------------------------------------------

class Calculator {
	class Token {
	public:
		char kind;
		double value;
		Token()
			: kind{char{}}, value{0} {}
		Token(char ch)
			: kind{ch}, value{0} {}
		Token(char ch, double val)
			: kind{ch}, value{val} {}
	};
	
	class Token_stream {
	public:
		Token_stream()
			: full{false}, buffer{Token{}} {}
		Token get();
		void putback(Token t);
	private:
		bool full;
		Token buffer;
	};

	double expression();
	double term();
	double primary();

	Token_stream ts;

public:
	double calculate_expression(const std::string& str);
};

//-----------------------------------------------------------------------------

void Calculator::Token_stream::putback(Token t)
{
	if (full)
		throw std::runtime_error("putback() into a full buffer");
	buffer = t;
	full = true;
}

//-----------------------------------------------------------------------------

Calculator::Token Calculator::Token_stream::get()
{
	if (full) {
		full = false;
		return buffer;
	}

	char ch;
	is>>ch;

	switch (ch) {
	case print:
	case '(': case ')': case '+': case '-': case '*': case '/': case '%':
		return Token{ch};
	case '.':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	{
		is.putback(ch);
		double val;
		is>>val;
		return Token{number,val};
	}
	default:
		throw std::runtime_error("Bad token");
	}
}

//-----------------------------------------------------------------------------

double Calculator::expression()
{
	double left = term();
	Token t = ts.get();

	while (true) {
		switch (t.kind) {
		case '+':
			left += term();
			t = ts.get();
			break;
		case '-':
			left -= term();
			t = ts.get();
			break;
		default:
			ts.putback(t);
			return left;
		}
	}
}

//-----------------------------------------------------------------------------

double Calculator::term()
{
	double left = primary();
	Token t = ts.get();

	while (true) {
		switch (t.kind) {
		case '*':
			left *= primary();
			t = ts.get();
			break;
		case '/':
		{
			double d = primary();
			if (d==0)
				throw std::runtime_error("divide by zero");
			left /= d;
			t = ts.get();
			break;
		}
		case '%':
		{
			int i1 = int(left);
			int i2 = int(term());

			if (i2==0)
				throw std::runtime_error("%: divide by zero");
			left = i1/i2;
			t = ts.get();
			break;
		}
		default:
			ts.putback(t);
			return left;
		}
	}
}

//-----------------------------------------------------------------------------

double Calculator::primary()
{
	Token t = ts.get();

	switch (t.kind) {
	case '(':
	{
		double d = expression();
		t = ts.get();
		if (t.kind!=')')
			throw std::runtime_error("')' expected");
		return d;
	}
	case number:
		return t.value;
	case '-':
		return -primary();
	case '+':
		return primary();
	default:
		throw std::runtime_error("primary expected");
	}
}

//-----------------------------------------------------------------------------

double Calculator::calculate_expression(const std::string& str)
{
	try {
		is = std::istringstream(str);
		
		Token t = ts.get();
		while (t.kind == ';')
			t = ts.get();
		ts.putback(t);
		return expression();
	}
	catch (std::exception& e) {
		std::cerr<<"Error: "<<e.what();
	}
}

//-----------------------------------------------------------------------------

class Calculator_Window : public Fl_Window {
	// Variables:


	// Widgets:
	Fl_Input *txtInput;
	Fl_Output *lblOutput;

	Fl_Button *btnCalculate;
	Fl_Button *btnQuit;

	// Events:
	void quit(Fl_Widget *w);
	void calculate(Fl_Widget *w);

	// Callbacks:
	static void cb_quit(Fl_Widget *w, void *f);
	static void cb_calculate(Fl_Widget *w, void *f);

public:
	Calculator_Window(int w, int h, const char *t);
	template<typename T>
	std::string convert_to_str(const T& v);
};

//-----------------------------------------------------------------------------

Calculator_Window::Calculator_Window(int w, int h, const char *t) : Fl_Window{w,h,t}
{
	// Attache controls:
	this->begin();

	txtInput = new Fl_Input{90,20,100,30,"Command:"};
	lblOutput = new Fl_Output{90,70,100,30,"Result:"};

	btnCalculate = new Fl_Button{20,120,80,30,"Calculate"};
	btnCalculate->callback(cb_calculate,(void *)this);
	btnQuit = new Fl_Button{110,120,80,30,"Quit"};
	btnQuit->callback(cb_quit,(void *)this);

	this->end();
}

//-----------------------------------------------------------------------------

void Calculator_Window::calculate(Fl_Widget *w)
{
	Calculator cal;
	double result = 0;
	std::string expression = txtInput->value();
	expression += print;

	result = cal.calculate_expression(expression);
	lblOutput->value(convert_to_str(result).c_str());
}

//-----------------------------------------------------------------------------

void Calculator_Window::quit(Fl_Widget *w)
{
	exit(0);
}

//-----------------------------------------------------------------------------

void Calculator_Window::cb_calculate(Fl_Widget *w, void *f)
{
	((Calculator_Window *)f)->calculate(w);
}

//-----------------------------------------------------------------------------

void Calculator_Window::cb_quit(Fl_Widget *w, void *f)
{
	((Calculator_Window *)f)->quit(w);
}

//-----------------------------------------------------------------------------

template<typename T>
std::string Calculator_Window::convert_to_str(const T& v)
{
	std::ostringstream os;
	os<<v;

	return os.str();
}

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	Calculator_Window *win = new Calculator_Window{230,170,"Calculator"};
	win->show(argc,argv);

	return Fl::run();
}
