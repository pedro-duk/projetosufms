#include <iostream>
#include <ctype.h>
#include <string>

using namespace std;

// Vetor que armazena nomes dos atributos
static const string nomesEnum[] = {
	"UNDEF",            // Não definido
	"FDA",              // Fim de arquivo
	"ID",               // Identificador
	"INTEGER_LITERAL",  // Inteiro literal
	"STRING_LITERAL",   // String literal

	"RELOP",    // Operadores
	"LT",       // <
	"GT",       // >
	"LE",       // <=
	"GE",       // >=
	"PLUS",     // +
	"MINUS",    // -
	"TIMES",    // *
	"DIV",      // /
	"MOD",      // %
	"AT",       // =
	"EQ",       // ==
	"NE",       // !=

	"SEP",      // Separadores
	"LPAREN",   // (
	"RPAREN",   // )
	"LSQUA",    // [
	"RSQUA",    // ]
	"LCURL",    // {
	"RCURL",    // }
	"SCOLON",   // ;
	"DOT",      // .
	"COMMA",    // ,

	// Palavras reservadas
	"CLASS",            // class
	"EXTENDS",          // extends
	"INTEGER",          // int
	"STRING",           // string
	"BREAK",            // break
	"PRINT",            // print
	"READ",             // read
	"RETURN",           // return
	"SUPER",            // super
	"IF",               // if
	"ELSE",             // else
	"FOR",              // for
	"NEW",              // new
	"CONSTRUCTOR"       // constructor
};

/*
// Vetor que armazena nomes dos atributos em português
static const string nomesEnumPT[] = {
	"Não definido",
	"Fim de Arquivo",
	"Identificador",
	"Inteiro literal",
	"String literal",

	"Operação lógica",
	"Menor que",
	"Maior que",
	"Menor ou igual a",
	"Maior ou igual a",
	"Mais",
	"Menos",
	"Vezes",
	"Dividido",
	"Resto",
	"Atribuição",
	"Igual a",
	"Diferente de",

	"Separadores",
	"Parênteses esquerdo",
	"Parênteses direito",
	"Parênteses esquerdo",
	"Parênteses esquerdo",
	"Parênteses esquerdo",
	"Parênteses esquerdo",
	"Ponto e vírgula",
	"Ponto",
	"Vírgula",

	// Palavras reservadas
	"Classe",           
	"Extends",
	"Inteiro",
	"String",
	"Break",
	"Print",
	"Read",
	"Return",
	"Super",
	"If",
	"Else",
	"For",
	"New",
	"Constructor"
};
*/

// Enum para os nomes dos atributos
enum Names 
{
	UNDEF,            // Não definido
	FDA,              // Fim de arquivo
	ID,               // Identificador
	INTEGER_LITERAL,  // Inteiro literal
	STRING_LITERAL,   // String

	RELOP,    // Operadores
	LT,       // Menor
	GT,       // Maior
	LE,       // Menor ou igual
	GE,       // Maior ou igual
	PLUS,     // Mais
	MINUS,    // Menos
	TIMES,    // Vezes
	DIV,      // Divisão
	MOD,      // Módulo
	AT,       // Atribuição
	EQ,       // Igualdade
	NE,       // Desigualdade

	SEP,      // Separadores    
	LPAREN,   // (
	RPAREN,   // )
	LSQUA,    // [
	RSQUA,    // ]
	LCURL,    // {
	RCURL,    // }
	SCOLON,   // ;
	DOT,      // .
	COMMA,    // ,

	// Palavras reservadas
	CLASS,
	EXTENDS,
	INTEGER,
	STRING,
	BREAK,
	PRINT,
	READ,
	RETURN,
	SUPER,
	IF,
	ELSE,
	FOR,
	NEW,
	CONSTRUCTOR
};

class Token 
{
	public: 
		int name;
		int attribute;
		string lexeme;
	
		Token(int name);
		Token(int name, string l);
		Token(int name, int attr);

		void print();
};