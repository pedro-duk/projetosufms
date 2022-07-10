#include "parser.h"

using namespace std;

int main (int argc, char* argv[]) {
	if(argc != 2) {
		cout << "Número de argumentos inválido. Uso: ./xpp_compiler teste1.xpp";
		exit(EXIT_FAILURE);
	}

	// Main
	Parser* parser = new Parser(argv[1]);

	try {
		parser->run();
	}

	catch(exception& e) {
		delete parser;
		exit(EXIT_FAILURE);
	}

	// Desalocações de memória	
	delete parser;

	// FIM
	return 0;
}