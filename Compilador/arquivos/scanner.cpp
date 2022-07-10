#include "scanner.h"

using namespace std;


/*---------------------
  Funções secundárias
---------------------*/

//Construtor
Scanner::Scanner(string input, SymbolTable* table) {
	this->input = "";

	// Abrindo arquivo
	ifstream inputFile(input, ios::in);
	string line;

	if(inputFile.is_open()) {
		while (getline(inputFile, line)) {
			this->input.append(line + '\n');
		}

		inputFile.close();
	}

	else {
		cout << "Erro na leitura do arquivo!\n";
		exit(EXIT_FAILURE);
	}

	// Inicializações
	st = table;
	ultpos = 0;
	pos = 0;
	linha = 1;
	posLinha = 0;
}

// Destrutor
Scanner::~Scanner() {

}

// Getter de linha
int Scanner::getLinha() {
	return linha;
}

// Getter de posLinha
int Scanner::getPosLinha() {
	return posLinha;
}

// Atualiza os atributos linha e posLinha. Chamado quando há erros. Retorna string.
string Scanner::definePosicao() {
	linha = 1;
	posLinha = 0;
	
	for(int i = 0; i < ultpos; i++) {
		if(input[i] == '\n') {
			linha++;
			posLinha = 0;
		}

		else {
			posLinha++;
		}
	}

	int posFinal = ultpos;	// Usado para capturar substring representando a linha.

	while(input[posFinal] != '\n') {
		posFinal++;
	}
	
	return input.substr(ultpos - posLinha, posFinal - ultpos + posLinha);
}

// Erro léxico
void Scanner::erroLexico(string msg) {
	string str = this->definePosicao();

	cout << "\n*** ERRO LÉXICO ***\nLinha "<< linha << ": " << msg << endl;
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

	cout << endl << "Compilação abortada por erro léxico. \n";

	throw ErroLexicoException();
}

/*------------------
  Função principal
------------------*/

// Método que retorna o próximo token da entrada
Token* Scanner::nextToken() {
	Token* tok;
	string lexeme = "";

	// Roda até receber retorno, ou gerar erro.
	while (true) {
		// Guarda a posição anterior, usado em erros
		ultpos = pos;

		/*----------------------------
		  IDENTIFICADORES E INTEIROS
		----------------------------*/

		// Checando IDs
		if (isalpha(input[pos])) {
			lexeme.push_back(input[pos]);
			pos++;
			
			while (isalpha(input[pos]) || isdigit(input[pos])) {
				lexeme.push_back(input[pos]);
				pos++;
			}

			// Checando palavras reservadas (pesquisando na tabela de símbolos global)
			STEntry* obj = st->get(lexeme);

			if(!obj) {	// É identificador!
				tok = new Token(ID, lexeme);
				return tok;
			}

			else { // É palavra reservada, ou variável global (classe)
				if(obj->reserved) {
					tok = new Token(obj->token->name);	// Palavra reservada!
					return tok;
				}

				else {
					tok = new Token(ID, lexeme);		// Variável global!
					return tok;
				}
			}
		}

		// Checando inteiros
		else if (isdigit(input[pos])) {

			// Adicionando todos os dígitos
			while (isdigit(input[pos])) {
				lexeme.push_back(input[pos]);
				pos++;
			}
			
			tok = new Token (INTEGER_LITERAL, lexeme);
			return tok;
		}


		/*------------
		  OPERADORES
		------------*/

		// Checando >= ou >
		else if (input[pos] == '>') {
			pos++;

			if (input[pos] == '=') {
				pos++;
				tok = new Token (RELOP, GE);    // Maior ou igual
				return tok;
			}

			else {
				tok = new Token (RELOP, GT);    // Maior
				return tok;
			}
		}

		// Checando <= ou <
		else if (input[pos] == '<') {
			pos++;

			if (input[pos] == '=') {
				pos++;
				tok = new Token (RELOP, LE);    // Menor ou igual
				return tok;
			}

			else {
				tok = new Token (RELOP, LT);    // Menor
				return tok;
			}
		}

		// Checando +
		else if (input[pos] == '+') {
			pos++;
			tok = new Token (RELOP, PLUS);      // Mais
			return tok;
		}

		// Checando -
		else if (input[pos] == '-') {
			pos++;
			tok = new Token (RELOP, MINUS);     // Menos
			return tok;
		}

		// Checando *
		else if (input[pos] == '*') {
			pos++;
			tok = new Token (RELOP, TIMES);     // Vezes
			return tok;
		}

		// Checando / e comentários
		else if (input[pos] == '/') {
			pos++;

			// Comentário de linha
			if(input[pos] == '/') {             
				// Ignora todos os caracteres até achar um '\n' ou '\n'
				do {
					pos++;
				} while (input[pos] != '\n' && input[pos] != '\0');

				// Avança para o próximo caractere, caso o atual não seja EOF
				if(input[pos] != '\0') {
					pos++;
				}

				// Próxima iteração do loop
				continue;
			}



			// Comentário de bloco
			else if(input[pos] == '*') {        
				// Ignora todos os caracteres até achar um '*' seguido de um '/'
				while(1) {
					pos++;

					if(input[pos] == '\0') break;

					if(input[pos] == '*' && input[pos + 1] == '/') {
						pos += 2;
						break;
					}
				}

				// Próxima iteração do loop
				continue;
			}

			else {
				tok = new Token (RELOP, DIV);   // Divisão
				return tok;
			}
		}

		// Checando %
		else if (input[pos] == '%') {
			tok = new Token (RELOP, MOD);       // Módulo
			return tok;
		}

		// Checando == ou =
		else if (input[pos] == '=') {
			pos++;

			if(input[pos] == '=') {
				pos++;
				tok = new Token(RELOP, EQ);     // Igualdade
				return tok;
			}

			else {
				tok = new Token(RELOP, AT);     // Atribuição
				return tok;
			}
			
		}

		// Checando !=
		else if(input[pos] == '!') {
			pos++;

			if(input[pos] == '=') {
				pos++;
				tok = new Token(RELOP, NE);     // Desigualdade
				return tok;
			}

			else { // TODO: Tratar not?
				this->erroLexico("Caractere ! não foi seguido de caractere =");
			}
		}

		/*-------------
		  SEPARADORES
		-------------*/

		else if(input[pos] == '(') {
			pos++;
			tok = new Token(SEP, LPAREN);         // Parênteses esquerdo
			return tok;
		}

		else if(input[pos] == ')') {
			pos++;
			tok = new Token(SEP, RPAREN);         // Parênteses direito
			return tok;
		}

		else if(input[pos] == '[') {
			pos++;
			tok = new Token(SEP, LSQUA);         // Colchete esquerdo
			return tok;
		}

		else if(input[pos] == ']') {
			pos++;
			tok = new Token(SEP, RSQUA);         // Colchete direito
			return tok;
		}

		else if(input[pos] == '{') {
			pos++;
			tok = new Token(SEP, LCURL);         // Chave esquerda
			return tok;
		}

		else if(input[pos] == '}') {
			pos++;
			tok = new Token(SEP, RCURL);         // Chave direita
			return tok;
		}

		else if(input[pos] == ';') {
			pos++;
			tok = new Token(SEP, SCOLON);        // Semicolon
			return tok;
		}

		else if(input[pos] == '.') {
			pos++;
			tok = new Token(SEP, DOT);           // Ponto
			return tok;
		}

		else if(input[pos] == ',') {
			pos++;
			tok = new Token(SEP, COMMA);         // Vírgula
			return tok;
		}

		/*--------
		  STRING
		--------*/

		else if(input[pos] == '"') {
			pos++; 

			while(input[pos] != '"' && input[pos] != '\0') {
				lexeme.push_back(input[pos]);
				pos++;
			}
			
			if(input[pos] == '"') {
				pos++;
				tok = new Token(STRING_LITERAL, lexeme);
				return tok;
			}

			else { // input[pos] == '\0', erro!
				this->erroLexico("Faltando caractere \" terminal!");
			}
		}

		/*--------
		  OUTROS
		--------*/
		
		// Consumindo espaços, tomando cuidado para acrescentar número de linhas em \n!
		else if (isspace(input[pos])) {
			while (isspace(input[pos])) {
				pos++;
			}

			// Próxima iteração do loop
			continue;
		}

		// EOF
		else if(input[pos] == '\0') {
			tok = new Token(FDA);
			return tok;
		}

		// Caractere não reconhecido!
		else {
			this->erroLexico("Caractere não reconhecido!");
		}
	}
}