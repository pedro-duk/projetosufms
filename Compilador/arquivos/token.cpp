#include "token.h"

using namespace std;

// Construtores
Token::Token(int name) {
	this->name = name;
	attribute = UNDEF;
	lexeme = "";
}

Token::Token(int name, string l) {
	this->name = name;
	attribute = UNDEF;
	lexeme = l;
}
		
Token::Token(int name, int attr) {
	this->name = name;
	attribute = attr;
	lexeme = "";
}

// Função para imprimir informações do token
void Token::print()
{
	cout << nomesEnum[name];

	if (attribute != UNDEF) {
		cout << "(" << nomesEnum[attribute] << ")";
	}

	cout << " ";
}