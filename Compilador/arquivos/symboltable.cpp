#include "symboltable.h"

// Construtor que instancia uma nova tabela sem especificar uma tabela pai.
SymbolTable::SymbolTable() {
    symbols = new map<string, STEntry*>();
    parent = 0;
}

// Construtor que instancia uma nova tabela especificando uma tabela pai.
SymbolTable::SymbolTable(SymbolTable* p) {
   	symbols = new map<string, STEntry*>();
	parent = p;
}

// Destrutor
SymbolTable::~SymbolTable() {
	// Removendo todas STEntry na tabela de símbolos
	auto it = symbols->begin();

	while(it != symbols->end()) {
		delete it->second;
		it++;
	}	

	// Limpando e removendo tabela de símbolos
    symbols->clear();

    delete symbols;
}

// Tenta adicionar um novo símbolo à tabela corrente.
bool SymbolTable::add(STEntry* t) {
    if (!(symbols->find(t->lexeme) == symbols->end())) {
        return false;
    }
    
    symbols->insert({t->lexeme, t});
    return true;
}

// Tenta remover um símbolo da tabela.
bool SymbolTable::remove(string name) {
    return symbols->erase(name) != 0;
}

// Verifica se a tabela está vazia.
bool SymbolTable::isEmpty() {
    return symbols->empty();
}

// Busca uma entrada equivalente ao lexema passado como parâmetro (subindo no escopo).
STEntry* SymbolTable::get(string name) {
    SymbolTable* table = this;

    auto s = table->symbols->find(name);

    while (s == table->symbols->end()) 	// Não achou!
    {
        table = table->parent;			// Na tabela pai, table->parent == 0!
        if (table == 0)					// Vasculhou todas as tabelas e não achou.
            return 0;

        s = table->symbols->find(name);
    } 
    
    return s->second;	// s é um iterator para o elemento achado. Second retorna STEntry*.
}

// Busca uma entrada equivalente ao lexema passado como parâmetro SEM SUBIR NO ESCOPO!
STEntry* SymbolTable::localGet(string name) {
    SymbolTable* table = this;

    auto s = table->symbols->find(name);

    if (s == table->symbols->end()) {
    	return 0;
    }

    else {
    	return s->second;
    }
}



// Retorna o pai da tabela atual.
SymbolTable* SymbolTable::getParent() {
    return parent;
}
