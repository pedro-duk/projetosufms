#include "scanner.h"

using namespace std;

// Exceção para tratamento de erros sintáticos
struct ErroSintaticoException : public exception {
   const char * what () const throw () {
      return "Erro Sintático";
   }
};

class Parser
{
	private:
	  	Scanner* scanner;
		Token* lToken;
		SymbolTable* globalST;
    	SymbolTable* currentST;
    	map<string, SymbolTable*>* classesMap; // Armazena as tabelas de símbolos de cada classe!

		bool isType(Token*);
		void advance(bool);
		void match(int);
		void match(int, bool);
		void erroSintatico(string);
		void erroSemantico(string);
		void initSymbolTable();
		void declaraID(string);
		bool acessaID();
		void criaTabSimb();
		void deletaTabSimb();

	  	// Funções da Gramática
		void Program();					// 1 e 2
		void ClassList();				// 3
		void ClassListLinha();			// 4 e 5
		void ClassDecl();				// 6
		void ClassDeclLinha(bool);		// 7 e 8
		void ClassBody();				// 9
		void ThingsOne();				// 10 a 12
		void ThingsOneAux(string);		// 13 e 14
		void ThingsOneLinha(string);	// 15 e 16
		void ThingsTwo();				// 17 a 19
		void ThingsTwoLinha(string);	// 20 e 21
		void ThingsThree();				// 22 e 23
		void VarDeclOpt(string);		// 24 e 25
		void MethodBody();				// 26
		string Type();					// 27 a 29
		void ParamListOpt();			// 30 e 31
		void ParamList();				// 32
		void ParamListLinha();			// 33 e 34
		void Param();					// 35
		void ParamLinha(string);		// 36 e 37
		void StatementsOpt();			// 38 e 39
		void Statements();				// 40
		void StatementsLinha();			// 41 e 42
		void Statement();				// 43 a 51
		void VarDeclAtribStat();		// 52 a 54
		void DeclAtribDecisao(string);		// 55 a 57
		void DeclAtribDecisaoLinha(string);	// 58 e 59
		void VarDeclLinha(string);			// 60 e 61
		void AtribStatLinha(string);	// 62 e 63
		void PrintStat();				// 64
		void ReadStat();				// 65
		void ReturnStat();				// 66
		void SuperStat();				// 67
		void IfStat();					// 68
		void IfStatLinha();				// 69 e 70
		void ForStat();					// 71
		void AtribStatOpt();			// 72 e 73
		void ExpressionOpt();			// 74 e 75
		void LValueComp();				// 76 e 77
		void LValueCompLinha();			// 78 e 79
		string Expression();				// 80
		void ExpressionLinha(string);		// 81 e 82
		string AllocExpression();			// 83 e 84
		string NumExpression();				// 85
		void NumExpressionLinha(string);	// 86 a 88
		string Term();						// 89
		void TermLinha(string);				// 90 a 93
		string UnaryExpression();			// 94 e 95
		string Factor();					// 96 a 99
		void ArgListOpt();				// 100 e 101
		void ArgList();					// 102
		void ArgListLinha();			// 103 e 104

	public:
		// Construtor
		Parser(string);

		// Destrutor
		~Parser();

		// Executa
		void run();
};
