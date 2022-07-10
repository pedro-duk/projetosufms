#include <map>
#include "stentry.h"

// Classe que representa uma tabela de símbolos.
class SymbolTable {
	private:
		SymbolTable* parent;			// Referência à tabela pai (escopo imediatamente anterior).
		
	public:
		map<string, STEntry*>* symbols; // Armazena os símbolos do escopo corrente.
		SymbolTable();				// Cria sem pai
		SymbolTable(SymbolTable*);  // Cria com pai
		~SymbolTable();				// Destrutor

		bool add(STEntry*);
		bool remove(string);
		bool isEmpty();
		STEntry* get(string);
		STEntry* localGet(string);
		SymbolTable* getParent();
};
