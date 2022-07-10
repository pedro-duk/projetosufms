#include "parser.h"

using namespace std;

bool erros = false;

/*---------------------
  Funções secundárias
---------------------*/

// Construtor
Parser::Parser(string input) {
	currentST = globalST = new SymbolTable();
	classesMap = new map<string, SymbolTable*>();
	initSymbolTable();

	scanner = new Scanner(input, globalST);
	lToken = NULL;
}

// Destrutor
Parser::~Parser() {
	delete scanner;
	delete lToken;
	delete globalST;

	// Removendo todas tabelas de símbolos de classes
	auto it = classesMap->begin();

	while(it != classesMap->end()) {
		delete it->second;
		it++;
	}

	// Limpando e removendo tabela de símbolos
    classesMap->clear();
    delete classesMap;
}

// Inicialização da tabela de símbolos
void Parser::initSymbolTable() {
	Token* t;

	t = new Token(CLASS);
	globalST->add(new STEntry(t, "class", true));
	t = new Token(EXTENDS);
	globalST->add(new STEntry(t, "extends", true));
	t = new Token(INTEGER);
	globalST->add(new STEntry(t, "int", true));
	t = new Token(STRING);
	globalST->add(new STEntry(t, "string", true));
	t = new Token(BREAK);
	globalST->add(new STEntry(t, "break", true));
	t = new Token(PRINT);
	globalST->add(new STEntry(t, "print", true));
	t = new Token(READ);
	globalST->add(new STEntry(t, "read", true));
	t = new Token(RETURN);
	globalST->add(new STEntry(t, "return", true));
	t = new Token(SUPER);
	globalST->add(new STEntry(t, "super", true));
	t = new Token(IF);
	globalST->add(new STEntry(t, "if", true));
	t = new Token(ELSE);
	globalST->add(new STEntry(t, "else", true));
	t = new Token(FOR);
	globalST->add(new STEntry(t, "for", true));
	t = new Token(NEW);
	globalST->add(new STEntry(t, "new", true));
	t = new Token(CONSTRUCTOR);
	globalST->add(new STEntry(t, "constructor", true));
}

// Checa se é tipo
bool Parser::isType(Token* tok) {
	if(tok->name == INTEGER || tok->name == STRING || tok->name == ID) {
		return true;
	}

	else {
		return false;
	}
}

// Erros sintáticos
void Parser::erroSintatico(string msg)
{
	string str = scanner->definePosicao();

	int linha = scanner->getLinha();
	int posLinha = scanner->getPosLinha();

	cout << "\n*** ERRO SINTÁTICO ***\nLinha "<< linha << ": " << msg << "." << endl;
	cout << linha << " |  " << str << endl;

	for(int i = 0; i < to_string(linha).size() + 4; i++) {
		cout << " ";
	}

	for(int i = 0; i < posLinha; i++) {
		if(str[i] == '\t') {
			cout << "\t";
		}

		else {
			cout << " ";
		}
	}
	
	cout << "^" << endl;

	cout << endl << "Compilação abortada por erro sintático. \n";

	throw ErroSintaticoException();
}

// Erros semânticos
void Parser::erroSemantico(string msg) {
	erros = true;

	string str = scanner->definePosicao();

	int linha = scanner->getLinha();
	int posLinha = scanner->getPosLinha();

	cout << "\n*** ERRO SEMÂNTICO ***\nLinha "<< linha << ": " << msg << "." << endl;
	cout << linha << " |  " << str << endl;

	for(int i = 0; i < to_string(linha).size() + 4; i++) {
		cout << " ";
	}

	for(int i = 0; i < posLinha; i++) {
		if(str[i] == '\t') {
			cout << "\t";
		}

		else {
			cout << " ";
		}
	}
	
	cout << "^" << endl;
}

// Obtém próximo token
void Parser::advance(bool deletarToken) {
	if (deletarToken) {
		delete lToken;
	}

	lToken = scanner->nextToken();
}

// Espera um token de tipo t, sempre deletando o token
void Parser::match(int t) {
	// Verificação
	if (lToken->name == t || lToken->attribute == t) {
		advance(true);
	}
	else {
		erroSintatico("Era esperado token do tipo " + nomesEnum[t]);
	}
}

// Espera um token de tipo t, podendo não deletar o token
void Parser::match(int t, bool deletarToken) {
	if (lToken->name == t || lToken->attribute == t) {
		advance(deletarToken);
	}
	else {
		erroSintatico("Era esperado token do tipo " + nomesEnum[t]);
	}
}


// Função chamada em declarações de IDs
void Parser::declaraID(string tipoAssociado) {
	if (lToken->name != ID) {
		match(ID);	// Irá gerar um erro sintático
	}

	// Garantindo que não existe variáveis com mesmo nome no escopo local
	else if (currentST->localGet(lToken->lexeme) != 0) {
		erroSemantico(lToken->lexeme + " previamente declarado no mesmo escopo");
		match(ID); // Continua execução
	}

	// Tudo certo
	else {
		/* 
		Não precisa disso, pois se tipo não existir já foi checado em Type
		// Verificando se tipoAssociado existe! Ele deve ser CLASSE, ou INTEGER ou STRING
		if (tipoAssociado != "INTEGER" && tipoAssociado != "STRING" && classesMap->find(tipoAssociado) == classesMap->end()) { // Não existe!
			erroSemantico(tipoAssociado + " não foi declarado anteriormente");
		}
		*/

		// Se classe não existir, é adicionada mesmo assim.
		currentST->add(new STEntry(lToken, lToken->lexeme, false, tipoAssociado));
		match(ID, false); // Não deleta token!
	}
}

// Função chamada em acesso de IDs
bool Parser::acessaID() {
	if (lToken->name != ID) {
		match(ID);	// Irá gerar um erro sintático
		return false;
	}

	else {
		STEntry* aux = currentST->get(lToken->lexeme);

		if(aux == 0) { // Não achou lexema em nenhuma tabela acessível!
			erroSemantico(lToken->lexeme + " não foi declarado anteriormente");
			match(ID); // Continua execução
			return false;
		}

		else { // Lexema encontrado
			match(ID);
			return true;
		}
	}
}

// Criação de uma nova tabela de símbolos
void Parser::criaTabSimb() {
	currentST = new SymbolTable(currentST);
}

// Deleta a tabela de símbolos atual
void Parser::deletaTabSimb() {
	if(currentST != 0) {
		SymbolTable* aux = currentST;
		currentST = currentST->getParent();
		delete aux;
	}
}

// Função principal, inicia o processamento
void Parser::run() {
	advance(false);

	Program();


	if (!erros) {
		cout << endl << "Compilação encerrada com sucesso! \n";
	}

	else {
		cout << endl << "Compilação encerrada com erros semânticos. \n";
	}
}

 
/*----------------------
  Funções da gramática
----------------------*/

// 1 e 2
void Parser::Program() {
	if(lToken->name != FDA) {
		ClassList();
	}

	if(lToken->name != FDA) {
		erroSintatico("Esperado token FIM DE ARQUIVO");
	}
}

// 3
void Parser::ClassList() {
	ClassDecl();
	ClassListLinha();
}

// 4 e 5
void Parser::ClassListLinha() {
	if(lToken->name == CLASS) {
		ClassList();
	}
}

// 6
void Parser::ClassDecl() {
	match(CLASS);

	if (lToken->name != ID) {
		match(ID);	// Irá gerar um erro sintático
	}

	// Procura se classe já existe
	if (classesMap->find(lToken->lexeme) == classesMap->end()) { // Não existe!
		// Nova tabela de símbolos para a classe!
		SymbolTable* novaST = new SymbolTable(globalST);
		currentST = novaST;

		// Inserindo tabela de símbolos da classe em classesMap
		classesMap->insert({lToken->lexeme, novaST});

		// Insere o mesmo lexema em globalST
		globalST->add(new STEntry(lToken, lToken->lexeme, false, nomesEnum[CLASS])); 
		match(ID, false); // Não deleta token!

		// Chamar ClassDeclLinha avisando pra pegar atributos de extends!
		ClassDeclLinha(true);
	}

	else { // Classe já existe, erro semântico
		erroSemantico("Redeclaração de classe " + lToken->lexeme);
		match(ID); // Continua execução

		// Fazer currentST apontar para classe já existente!
		currentST = (classesMap->find(lToken->lexeme))->second;

		// Chamar ClassDeclLinha avisando pra NÃO pegar atributos de extends!
		ClassDeclLinha(false);
	}

	// Após declaração de uma classe, currentST pode ser setado como globalST.
	currentST = globalST;
}

// 7 e 8
void Parser::ClassDeclLinha(bool herdarExtends) {
	if(lToken->attribute == LCURL) {
		ClassBody();
	}

	else if (lToken->name == EXTENDS) {
		match(EXTENDS);

		// Acessando classe!

		if (lToken->name != ID) {
			match(ID);	// Irá gerar um erro sintático
		}

		else { // Próximo é um ID
			STEntry* aux = currentST->get(lToken->lexeme); // Classe declarada anteriormente

			if(aux == 0) { // Não achou lexema em nenhuma tabela acessível!
				erroSemantico(lToken->lexeme + " não foi declarado anteriormente");
				match(ID); // Continua execução
			}

			else { // Classe declarada anteriormente, tudo certo!
				if(herdarExtends) {
					// Adicionar em currentST todas as entradas da classe achada!
					// Essa classe está acessível por lToken->lexeme
					SymbolTable* herdada = (classesMap->find(lToken->lexeme))->second;

					auto it = (herdada->symbols)->begin();

					while(it != (herdada->symbols)->end()) { // it->second devolve um STEntry*
						currentST->add(new STEntry((it->second)->token, (it->second)->lexeme, false, (it->second)->tipoAssociado));
						it++;
					}
				}
				match(ID);
			}
		}

		ClassBody();
	}

	else {
		erroSintatico("Classe declarada incorretamente");
	}
}

// 9
void Parser::ClassBody() {
	match(LCURL);
	ThingsOne();
	match(RCURL);
}

// 10 a 12
void Parser::ThingsOne() {
	if(isType(lToken)){
		string aux = Type();
		ThingsOneAux(aux);
	}

	else if(lToken->name == CONSTRUCTOR) {
		match(CONSTRUCTOR);
		MethodBody();
		ThingsTwo();
	}
}

// 13 e 14
void Parser::ThingsOneAux(string tipo) {
	if(lToken->name == ID) {
		declaraID(tipo);
		ThingsOneLinha(tipo);
	}

	else if (lToken->attribute == LSQUA) {
		match(LSQUA);
		match(RSQUA);
		declaraID(tipo);
		ThingsOneLinha(tipo);
	}

	else {
		erroSintatico("Declaração de metodo ou atributo inválida");
	} 
}

// 15 e 16
void Parser::ThingsOneLinha(string tipo) {
	if(lToken->attribute == COMMA || lToken->attribute == SCOLON) {
		VarDeclOpt(tipo);
		match(SCOLON);
		ThingsOne();
	}

	else if (lToken->attribute == LPAREN) {
		MethodBody();
		ThingsThree();
	}

	else {
		erroSintatico("Variável ou método declarado incorretamente");
	}
}

// 17 a 19
void Parser::ThingsTwo() {
	if(lToken->name == CONSTRUCTOR) {
		match(CONSTRUCTOR);
		MethodBody();
		ThingsTwo();
	}

	else if(isType(lToken)){
		string aux = Type();
		ThingsTwoLinha(aux);
	}
}

// 20 e 21
void Parser::ThingsTwoLinha(string tipo) {
	if(lToken->name == ID) {
		declaraID(tipo);
		MethodBody();
		ThingsThree();
	}

	else if (lToken->attribute == LSQUA) {
		match(LSQUA);
		match(RSQUA);
		declaraID(tipo);
		MethodBody();
		ThingsThree();
	}

	else {
		erroSintatico("Método declarado incorretamente");
	}
}

// 21 a 23
void Parser::ThingsThree() {
	if(isType(lToken)){
		string aux = Type();
		ThingsTwoLinha(aux);
	}
}

// 24 e 25
void Parser::VarDeclOpt(string tipo) {
	if(lToken->attribute == COMMA) {
		match(COMMA);
		declaraID(tipo);
		VarDeclOpt(tipo);
	}
}

// 26
void Parser::MethodBody() {
	criaTabSimb();

	match(LPAREN);
	ParamListOpt();
	match(RPAREN);
	match(LCURL);
	StatementsOpt();
	match(RCURL);

	deletaTabSimb();
}

// 27 a 29
string Parser::Type() {
	if(isType(lToken)) {
		if(lToken->name == ID) {
			string lex = lToken->lexeme;

			// Acesso a IDs sem esperar um tipo!
			// Já faz match
			acessaID();

			// Não verifica se tipo existe! Isso é feito por declaraID.
			return lex;
		}

		else {
			int aux = lToken->name;
			match(lToken->name);
			return nomesEnum[aux];
		}
	}

	else {
		erroSintatico("Tipo inválido");
		return "";
	}
}

// 30 e 31
void Parser::ParamListOpt() {
	if(isType(lToken)) {
		ParamList();
	}
}

// 32
void Parser::ParamList() {
	Param();
	ParamListLinha();
}

// 33 e 34
void Parser::ParamListLinha() {
	if(lToken->attribute == COMMA) {
		match(COMMA);
		Param();
		ParamListLinha();
	}
}

// 35
void Parser::Param() {
	string aux = Type();
	ParamLinha(aux);
}

// 36 e 37
void Parser::ParamLinha(string tipo) {
	if(lToken->name == ID) {
		declaraID(tipo);
	}

	else if(lToken->attribute == LSQUA) {
		match(LSQUA);
		match(RSQUA);
		declaraID(tipo);
	}

	else {
		erroSintatico("Parâmetro inválido");
	}
}

// 38 e 39
void Parser::StatementsOpt() {
	if(lToken->attribute != RCURL) {
		Statements();
	}
}

// 40
void Parser::Statements() {
	if(lToken->attribute == RCURL) {
		erroSintatico("Necessário ao menos uma sentença");
	}

	Statement();
	StatementsLinha();
}

// 41 e 42
void Parser::StatementsLinha() {
	if(lToken->attribute != RCURL) {
		Statement();
		StatementsLinha();
	}
}

// 43 a 51
void Parser::Statement() {
	if(isType(lToken)) {
		VarDeclAtribStat();
		match(SCOLON);
	}

	else if(lToken->name == PRINT) {
		PrintStat();
		match(SCOLON);
	}

	else if(lToken->name == READ) {
		ReadStat();
		match(SCOLON);
	}

	else if(lToken->name == RETURN) {
		ReturnStat();
		match(SCOLON);
	}

	else if(lToken->name == SUPER) {
		SuperStat();
		match(SCOLON);
	}

	else if(lToken->name == IF) {
		IfStat();
	}

	else if(lToken->name == FOR) {
		ForStat();
	}

	else if(lToken->name == BREAK) {
		match(BREAK);
		match(SCOLON);
	}

	else if(lToken->attribute == SCOLON) {
		match(SCOLON);
	}

	else {
		erroSintatico("Sentença inválida");
	}
}

// 52 a 54
void Parser::VarDeclAtribStat() {
	if(lToken->name == INTEGER) { // É uma declaração!
		match(INTEGER);
		VarDeclLinha(nomesEnum[INTEGER]);
	}

	else if(lToken->name == STRING) { // É uma declaração!
		match(STRING);
		VarDeclLinha(nomesEnum[STRING]);
	}

	else if(lToken->name == ID) { // Não temos certeza se é declaração ou atribuição...
		string lex = lToken->lexeme;
		acessaID(); // Verificando se tipo existe

		DeclAtribDecisao(lex);
	}

	else {
		erroSintatico("Declaração de variável ou atribuição inválida"); // Código inatingível
	}
}

// 55 a 57
void Parser::DeclAtribDecisao(string lexPassado) {
	if(lToken->attribute == DOT || lToken->attribute == AT) { // É uma atribuição!
		string valPassado = "-1";

		// Verificando se existe lexema
		STEntry* aux = currentST->get(lexPassado);

		if(aux != 0) { // Existe!
			valPassado = aux->tipoAssociado;
		} 

		LValueComp();
		match(AT);
		AtribStatLinha(valPassado);
	}
	else if(lToken->name == ID) { // É uma declaração!
		declaraID(lexPassado); // Verificando lexPassado, e já declarando o próximo ID com esse tipo!
		VarDeclOpt(lexPassado);
	}

	else if (lToken->attribute == LSQUA) { // Ainda não sabemos se é declaração ou atribuição...
		match(LSQUA);
		DeclAtribDecisaoLinha(lexPassado); // Lembrar de verificar se lexPassado existe!
	}

	else {
		erroSintatico("Declaração de variável ou atribuição inválida");
	}
}

// 58 e 59
void Parser::DeclAtribDecisaoLinha(string lexPassado) {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) { // É uma atribuição!
		string valPassado = "-1";

		Expression();
		match(RSQUA);

		// Verificando se existe lexema
		STEntry* aux = currentST->get(lexPassado);

		if(aux != 0) { // Existe!
			valPassado = aux->tipoAssociado;
		} 

		LValueComp();
		match(AT);
		AtribStatLinha(valPassado);
	}

	else if(lToken->attribute == RSQUA) { // É uma declaração!
		match(RSQUA);
		declaraID(lexPassado);
		VarDeclOpt(lexPassado);
	}

	else {
		erroSintatico("Declaração de variável ou atribuição inválida");
	}
}

// 60 e 61
void Parser::VarDeclLinha(string tipo) {
	if(lToken->name == ID) {
		declaraID(tipo);
		VarDeclOpt(tipo);
	}

	else if(lToken->attribute == LSQUA) {
		match(LSQUA);
		match(RSQUA);
		declaraID(tipo);
		VarDeclOpt(tipo);
	}

	else {
		erroSintatico("Declaração de variável inválida");
	}
}

// 62 e 63
void Parser::AtribStatLinha(string tipoEsquerda) {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) {
		string tipoDireita = Expression();

		if(tipoEsquerda != "-1" && tipoDireita != "-1" && tipoEsquerda != tipoDireita) {
			erroSemantico("Erro em atribuição: Era esperado tipo " + tipoEsquerda + ", obtido " + tipoDireita);
		}
	}
	
	else if(lToken->name == NEW || isType(lToken)) {
		string tipoDireita = AllocExpression();

		if(tipoEsquerda != "-1" && tipoDireita != "-1" && tipoEsquerda != tipoDireita) {
			erroSemantico("Erro em atribuição: Era esperado tipo " + tipoEsquerda + ", obtido " + tipoDireita);
		}
	}
	
	else {
		erroSintatico("Atribuição inválida");
	}
}

// 64
void Parser::PrintStat() {
	match(PRINT);
	Expression();
}

// 65
void Parser::ReadStat() {
	match(READ);
	acessaID(); // Certinho!
	LValueCompLinha();
}

// 66
void Parser::ReturnStat() {
	match(RETURN);
	Expression();
}

// 67
void Parser::SuperStat() {
	match(SUPER);
	match(LPAREN);
	ArgListOpt();
	match(RPAREN);
}

// 68
void Parser::IfStat() {
	match(IF);
	match(LPAREN);
	Expression();
	match(RPAREN);

	criaTabSimb();

	match(LCURL);
	Statements();
	match(RCURL);

	deletaTabSimb();

	IfStatLinha();
}

// 69 e 70
void Parser::IfStatLinha() {
	if(lToken->name == ELSE) {
		match(ELSE);

		criaTabSimb();

		match(LCURL);
		Statements();
		match(RCURL);

		deletaTabSimb();
	}
}

// 71
void Parser::ForStat() {
	match(FOR);
	match(LPAREN);
	AtribStatOpt();
	match(SCOLON);
	ExpressionOpt();
	match(SCOLON);
	AtribStatOpt();
	match(RPAREN);

	criaTabSimb();

	match(LCURL);
	Statements();
	match(RCURL);

	deletaTabSimb();
}

// 72 e 73
void Parser::AtribStatOpt() {
	if(lToken->name == ID) {
		string valPassado = "-1";

		string lex = lToken->lexeme;
		bool sucesso = acessaID();

		LValueCompLinha();
		match(AT);

		if(sucesso) {
			valPassado = currentST->get(lex)->tipoAssociado;
		}

		AtribStatLinha(valPassado);
	}
}

// 74 e 75
void Parser::ExpressionOpt() {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) {
		Expression();
	}

	else if (lToken->attribute != SCOLON) {
		erroSintatico("Expressão inválida (variáveis e valores precisam de sinal)");
	}
}

// 76 e 77
void Parser::LValueComp() {
	if(lToken->attribute == DOT) {
		match(DOT);
		match(ID);
		LValueCompLinha();
	}
}

// 78 e 79
void Parser::LValueCompLinha() {
	if(lToken->attribute == LSQUA) {
		match(LSQUA);
		Expression();
		match(RSQUA);
		LValueComp();
	}

	else {
		LValueComp();
	}
}

// 80
string Parser::Expression() {
	string aux = NumExpression();
	ExpressionLinha(aux);

	return aux;
}

// 81 e 82
void Parser::ExpressionLinha(string aux) {
	if(lToken->name == RELOP) {
		match(RELOP);
		string tipoDireitaRelop = NumExpression();

		if(aux != "-1" && tipoDireitaRelop != "-1" && aux != tipoDireitaRelop) { // Dois lados da RELOP não são do mesmo tipo
			erroSemantico("Operadores devem ser do mesmo tipo");
		}
	}
}

// 83 e 84
string Parser::AllocExpression() {
	if(lToken->name == NEW) {
		match(NEW);

		string aux = lToken->lexeme;
		if(lToken->name != ID) {
			aux = "-1";
		}

		acessaID();
		match(LPAREN);
		ArgListOpt();
		match(RPAREN);

		return aux;
	}

	else if(isType(lToken)) {
		string aux = Type();
		match(LSQUA);
		Expression();
		match(RSQUA);

		return aux;
	}

	else {
		erroSintatico("Erro na alocação de variável.");
		return "-1";
	}
}

// 85
string Parser::NumExpression() {
	string aux = Term();
	NumExpressionLinha(aux);
	return aux;
}

// 86 a 88
void Parser::NumExpressionLinha(string aux) {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) {
		match(lToken->attribute);
		string valterm = Term();

		if(aux != "-1" && valterm != "-1" && valterm != aux) {
			erroSemantico("Operadores devem ser do mesmo tipo");
		}
	}
}

// 89
string Parser::Term() {
	string aux = UnaryExpression();
	TermLinha(aux);
	return aux;
}

// 90 a 93
void Parser::TermLinha(string aux) {
	if(lToken->attribute == TIMES || lToken->attribute == DIV || lToken->attribute == MOD) {
		match(lToken->attribute);
		string valterm = UnaryExpression();

		if (valterm != aux) {
			erroSemantico("Operadores devem ser do mesmo tipo");
		}
	}
}

// 94 e 95
string Parser::UnaryExpression() {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) {
		match(lToken->attribute);
		string valterm = Factor();
		return valterm;
	}

	else {
		erroSintatico("Expressão unária inválida");
		return "-1";
	}
}

// 96 a 99
string Parser::Factor() {
	if (lToken->name == INTEGER_LITERAL || lToken->name == STRING_LITERAL) {
		int aux = lToken->name;
		match(lToken->name);

		if(aux == INTEGER_LITERAL) {
			return nomesEnum[INTEGER];
		}

		else {
			return nomesEnum[STRING];
		}
	}

	else if (lToken->name == ID) {
		string valRet = "-1";

		// Pegar tipo associado, não o lexema!
		string lex = lToken->lexeme;
		acessaID();

		STEntry* aux = currentST->get(lex);
		if (aux != 0) { // Achou tipo!
			valRet = aux->tipoAssociado;
		}

		LValueCompLinha();
		return valRet;
	}

	else if (lToken->attribute == LPAREN) {
		match(LPAREN);
		string val = Expression();
		match(RPAREN);

		return val;
	}

	else {
		erroSintatico("Fator inválido");
		return "-1";
	}
}

// 100 e 101
void Parser::ArgListOpt() {
	if(lToken->attribute == PLUS || lToken->attribute == MINUS) {
		ArgList();
	}

	else if(lToken->attribute != RPAREN) {
		erroSintatico("Argumento inválido");
	}
}

// 102
void Parser::ArgList() {
	Expression();
	ArgListLinha();
}

// 103 e 104
void Parser::ArgListLinha() {
	if(lToken->attribute == COMMA) {
		match(COMMA);
		Expression();
		ArgListLinha();
	}
}