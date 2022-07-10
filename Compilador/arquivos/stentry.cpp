#include "stentry.h"

STEntry::~STEntry() {
	delete token;
}

STEntry::STEntry() {
	reserved = false;
}
        
STEntry::STEntry(Token* tok, string lex) {
    token = tok;
    lexeme = lex;
    reserved = false;
}

STEntry::STEntry(Token* tok, string lex, bool res) {
    token = tok;
    lexeme = lex;
    reserved = res;
}

STEntry::STEntry(Token* tok, string lex, bool res, string tipo) {
    token = tok;
    lexeme = lex;
    reserved = res;
    tipoAssociado = tipo;
}