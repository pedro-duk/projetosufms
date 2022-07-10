#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string>
#include <exception>
#include "symboltable.h"

using namespace std;

// Exceção para tratamento de erros léxicos
struct ErroLexicoException : public exception {
   const char * what () const throw () {
      return "Erro Léxico";
   }
};

class Scanner 
{
	private: 
		string input;   	// Armazena o texto de entrada
		int pos;        	// Posição atual
		int ultpos;			// Posição do último token
		int linha;      	// Linha atual
		int posLinha;		// Posição relativo à linha atual
		SymbolTable* st;	// Tabela de símbolos. No parser, é inicializada como a global

		void fillTable();

	public:
		// Construtor
		Scanner(string, SymbolTable*);

		// Destrutor
		~Scanner();
	
		// Método que retorna o próximo token da entrada
		Token* nextToken();        
	
		// Método para manipular erros
		void erroLexico(string);

		// Chamada quando há erros. A partir de pos e input, calcula linha e posLinha. Retorna a string da linha.
		string definePosicao();

		// Getters
		int getLinha();
		int getPosLinha();
};
