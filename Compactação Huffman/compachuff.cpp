/***************************************************
 *
 * Pedro Luiz da Costa Silva
 * Trabalho 2
 * Professor: Diego Padilha Rubert
 *
 */

#include <stdio.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <string>
#include <map>
#include <math.h>
#include <bitset>

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::bitset; // Usado para converter char para string binária

typedef pair<unsigned char, string> hentry; 	// Entrada tabela HASH
typedef map<unsigned char, string> htable;		// Tabela HASH

/* Nó de uma árvore de Huffman */
class Node {
private:
	int f;     // Frequência
	char c;    // Código ASCII
	Node *l;   // Filho esquerdo
	Node *r;   // Filho direito
	
public:
	Node(int f, int c, Node *l = NULL, Node *r = NULL); // Construtor
	Node(int f = 0, Node *l = NULL, Node *r = NULL);    // Construtor
	~Node();											// Destrutor
	int freq(void);    // Devolve a frequência do caractere
	char code(void);   // Devolve o código do caractere
	Node *left(void);  // Devolve o filho esquerdo
	Node *right(void); // Devolve o filho direito
	void setleft(Node* no);		// Seta no como filho esquerdo
	void setright(Node* no);	// Seta no como filho direito
	void setcode(char c);		// Seta c como codigo ascii
	bool leaf(void);   // Devolve true se é folha e false caso contrário
};

/* Lista de min-prioridades (min-heap) com nós de uma árvore de
	 Huffman, utilizando a frequência como chave */
class MinHeap {
private:
	vector<Node *> v;  // Elementos
	void up(int i);    // Sobe
	void down(int i);  // Desce
	int parent(int i); // Pai
	int left(int i);   // Filho esquerdo
	int right(int i);  // Filho direito
	void troca(int x, int y); // Troca v.at(x) por v.at(y)

public:
	MinHeap();            // Construtor padrão
	~MinHeap();           // Destrutor padrão
	int size(void);       // Tamanho da heap
	void insert(Node *n); // Inserir elemento
	Node *extract(void);  // Remover (menor) elemento
};

//*********************
//*** FUNÇÕES EXTRA ***
//*********************

void caminhopreordem(Node* aux, string* strpreordem, string* codigohuff, htable* m, string* charspreordem){
	if(aux == NULL) {
		return;
	}

	if(aux->leaf()) {
		strpreordem->push_back('1');
		charspreordem->push_back(aux->code());
		m->insert(hentry(aux->code(), *codigohuff));
	}

	else {
		strpreordem->push_back('0');
	}

	codigohuff->push_back('0');
	caminhopreordem(aux->left(), strpreordem, codigohuff, m, charspreordem);
	codigohuff->pop_back();

	codigohuff->push_back('1');
	caminhopreordem(aux->right(), strpreordem, codigohuff, m, charspreordem);
	codigohuff->pop_back();
}

// Assume que aux tem 8 caracteres, somente 0 ou 1
char binarystringtochar(string aux) {
	int soma = 0;

	for(int i = 0; i < 8; i++) {
		if(aux.at(7 - i) == '1') {
			soma += (int)pow(2, i);
		}
	}

	return soma;
}

void escrevesaida(string* strsaida, FILE* arqsaida) {
	string stringaux;
	char charaux;

	while(strsaida->size() > 7) {
		stringaux = strsaida->substr(0, 8);
		charaux = binarystringtochar(stringaux);
		fwrite (&charaux, sizeof(char), 1, arqsaida);
		strsaida->erase(0, 8);
	}
}

void leumbyte(FILE* arquivo, string* strlido) {
	char charaux;
	fread(&charaux, sizeof(char), 1, arquivo);
	*strlido += bitset<8>(charaux).to_string();
}

void criaarvhuffman(Node* raizhuffman, string* strpreordem, string* letras) {
	if(raizhuffman != NULL && letras->size() > 0 && strpreordem->size() > 0) {
		if(strpreordem->at(0) == '1') { // Folha
			raizhuffman->setcode(letras->at(0));
			strpreordem->erase(0,1);
			letras->erase(0,1);
		}

		else {
			Node* left = new Node();
			Node* right = new Node();
			raizhuffman->setleft(left);
			raizhuffman->setright(right);

			strpreordem->erase(0,1);
			criaarvhuffman(left, strpreordem, letras);
			criaarvhuffman(right, strpreordem, letras);
		}
	}
}

char consultaarvhuffman(Node* raizhuffman, string consulta, int* flag) {
	string codigo;
	*flag = 0;

	for(unsigned int i = 0; i < consulta.size(); i++) {
		if(consulta.at(i) == '1') {
			raizhuffman = raizhuffman->right();
		}

		else {
			raizhuffman = raizhuffman->left();
		}

		if(raizhuffman == NULL) {
			*flag = -1;
			return 'a';
		}
	}

	if (raizhuffman->leaf()) {
		return raizhuffman->code();
	}

	else {
		*flag = -1;
		return 'a';
	}
}

//************
//*** MAIN ***
//************


int main(int argc, char **argv) {
	// Checa se número de argumentos é 4
	if(argc != 4) {
		printf("Argumentos invalidos.");
		return -1;
	}

	// Checa se segundo argumendo é 'c' ou 'd'
	if(strcmp(argv[1], "c") != 0 && strcmp(argv[1], "d") != 0) {
		printf("Argumentos invalidos.\n");
		return -1;
	}

	// Compactação!
	if(strcmp(argv[1], "c") == 0) {
		MinHeap minheap;	// Minheap para passo 2

		// Leitura do arquivo
		string strlido;
		char charaux;

		FILE *arquivo;
		arquivo = fopen(argv[2], "r");

		if(arquivo == NULL) { // Caso falhou em abrir, fecha o programa.
			printf("Arquivo falhou em abrir.\n");
			return -1;
		}

		while(fread(&charaux, sizeof(char), 1, arquivo) != 0) {
			strlido += charaux;
		}

		// Vetor de frequências
		int frequencias[256];
		for(int i = 0; i < 256; i++) {
			frequencias[i] = 0;
		}

		// PASSO 1: Contando frequências de cada letra
		unsigned int tam = strlido.size();
		for(unsigned int i = 0; i < tam; i++) {
			frequencias[(int)strlido.at(i)]++;
		}

		// PASSO 2: Criando min-heap com vetor de frequências
		for(int i = 0; i < 256; i++) {
			if(frequencias[i] > 0) {
				Node* aux = new Node(frequencias[i], i);
				minheap.insert(aux);
			}
		}

		// PASSO 3: Criando árvore de HUFFMAN
		while(minheap.size() > 1) {

			Node * auxesq = minheap.extract();
			Node * auxdir = minheap.extract();
			int auxfreq = auxesq->freq() + auxdir->freq();

			Node* aux = new Node(auxfreq, 0, auxesq, auxdir);
			
			minheap.insert(aux);
		}

		Node* raizhuffman = minheap.extract();

		// PASSO 4: Criação de Tabela HASH com códigos
		string strpreordem; 	// Usada no passo 5
		string charspreordem; 	// Usada no passo 5
		string codigohuff;		// Usada para guardar o código de Huffman correspondente
		htable tabhash;			// Tabela hash

		caminhopreordem(raizhuffman, &strpreordem, &codigohuff, &tabhash, &charspreordem);


		// PASSO 5: Geração do arquivo de saída!
		FILE* arqsaida;
		arqsaida = fopen(argv[3],"wb");


		// Escrevendo o tamanho do alfabeto!
		unsigned short tamanhoalfabeto = (short)tabhash.size();
		fwrite (&tamanhoalfabeto, sizeof(char), 2, arqsaida);

		// Escrevendo as letras do alfabeto em formato pré-ordem!
		const char* aux = charspreordem.c_str();
		fwrite (aux, sizeof(char), tamanhoalfabeto, arqsaida);

		// Escrevendo o alfabeto
		string strsaida; // String que guardará os bits de saída.

		strsaida += strpreordem; // Adicionando os bits do pré-ordem

		escrevesaida(&strsaida, arqsaida); // Escrevendo no arquivo, e removendo de strsaida

		tam = strlido.size();
		for(unsigned int i = 0; i < tam; i++) {
			strsaida += tabhash.find(strlido.at(i))->second;
			escrevesaida(&strsaida, arqsaida);
		}

		char qtdbits = 0;

		// Inserindo bits 0 extras...
		while(strsaida.size() < 8) {
			strsaida += "0";
			qtdbits++;
		}

		strsaida += bitset<8>(qtdbits).to_string();

		escrevesaida(&strsaida, arqsaida);

		delete raizhuffman;
		fclose(arquivo);
		fclose(arqsaida);
	}

	// Descompactação!
	else {
		char charaux;
		vector<char> lido;

		FILE *arquivo;
		arquivo = fopen(argv[2], "rb");

		if(arquivo == NULL) { // Caso falhou em abrir, fecha o programa.
			printf("Arquivo falhou em abrir.\n");
			return -1;
		}

		// PASSO 1: Leitura do cabeçalho
		// Lendo o tamanho do alfabeto
		unsigned short tamalfabeto;
		fread(&tamalfabeto, sizeof(short), 1, arquivo);

		// Lendo as letras do alfabeto e armazenando em uma string
		string stralfabeto;

		for (unsigned short i = 0; i < tamalfabeto; i++) {
			fread(&charaux, sizeof(char), 1, arquivo);
			stralfabeto.push_back(charaux);
		}

		// Lendo o percurso pré-ordem
		string strlido;
		string preordem;
		unsigned short restantes = tamalfabeto;
		unsigned int counter = 0;

		leumbyte(arquivo, &strlido);

		while(restantes > 0) {
			charaux = strlido.at(counter);
			preordem.push_back(charaux);
			counter++;

			if(charaux == '1') {
				restantes--;
			}

			if(counter >= strlido.size()){
				leumbyte(arquivo, &strlido);
			}
		}

		// Final: counter se encontra na posição do primeiro bit do texto compactado.
		// Cortaremos os bits que não precisamos mais.
		strlido.erase(0, counter);

		// PASSO 2: Árvore de Huffman
		Node* raizhuffman = new Node();

		criaarvhuffman(raizhuffman, &preordem, &stralfabeto);

		// PASSO 3: Descompactar texto
		// Leitura de todos os bytes restantes.
		char ultimochar;

		while(fread(&charaux, sizeof(char), 1, arquivo) != 0) {
			strlido += bitset<8>(charaux).to_string();
			ultimochar = charaux;
		}

		// Faremos com que strlido tenha somente os caracteres do texto.
		int qtdcorte = (int)ultimochar + 8;

		strlido.erase(strlido.size() - qtdcorte, qtdcorte);

		// Leitura de caractere a caractere, criando arquivo de saída
		string consulta;
		int flag;

		FILE* arqsaida;
		arqsaida = fopen(argv[3],"w");

		for(unsigned int i = 0; i < strlido.size(); i++) {
			consulta.push_back(strlido.at(i));
			char aux = consultaarvhuffman(raizhuffman, consulta, &flag);

			if(flag == 0) {
				fputc(aux, arqsaida);
				consulta = "";
			}
		}

		delete raizhuffman;
		fclose(arquivo);
		fclose(arqsaida);
	}

	return 0;
}



//*************************************
//*** IMPLEMENTAÇÕES DA CLASSE NODE ***
//*************************************


Node::Node(int f, int c, Node *l, Node *r) {
	this->c = c;
	this->f = f;
	this->l = l;
	this->r = r;
}

Node::Node(int f, Node *l, Node *r) {
	this->f = f;
	this->l = l;
	this->r = r;
}

Node::~Node() {
	if(l != NULL) {
		delete l;
	}

	if(r != NULL) {
		delete r;
	}
}

int Node::freq(void) {
	return this->f;
}

char Node::code(void) {
	return this->c;
}

Node* Node::left(void) {
	return this->l;
}

Node* Node::right(void) {
	return this->r;
}

void Node::setleft(Node* no) {
	this->l = no;
}

void Node::setright(Node* no) {
	this->r = no;
}

void Node::setcode(char c) {
	this->c = c;
}

bool Node::leaf(void) { 
	if(this->l == NULL && this->r == NULL) {
		return true;
	}

	else {
		return false;
	}
}

//****************************************
//*** IMPLEMENTAÇÕES DA CLASSE MINHEAP ***
//****************************************

void MinHeap::up(int i) {
	while (v.at(parent(i))->freq() > v.at(i)->freq()) {
		troca(i, parent(i));
		i = parent(i);
	}
}

void MinHeap::down(int i) {
	int e = left(i);
	int d = right(i);
	int n = size();

	int menor = i;

	if(e < n and v.at(e)->freq() < v.at(menor)->freq()) {
		menor = e;
	}

	if(d < n and v.at(d)->freq() < v.at(menor)->freq()) {
		menor = d;
	}

	if(menor != i) {
		troca(i, menor);
		down(menor);
	}
}

int MinHeap::parent(int i) {
	return (i - 1) / 2;
}

int MinHeap::left(int i) {
	return 2 * (i + 1) - 1;
}

int MinHeap::right(int i) {
	return 2 * (i + 1);
}

void MinHeap::troca(int x, int y) {
	Node* aux = v.at(x);
	v.at(x) = v.at(y);
	v.at(y) = aux;
}

MinHeap::MinHeap() {
	// Vazio!
}


MinHeap::~MinHeap() { 
	unsigned int tam = v.size();

	for(unsigned int i = 0; i < tam; i++) {
		delete v[i];
	}
}

int MinHeap::size(void) {
	return v.size();
}

void MinHeap::insert(Node *n) {
	v.push_back(n);
	up(size() - 1);
}

Node* MinHeap::extract(void) {
	int n = size();

	if(n > 0) {
		Node* menor = v.front();
		troca(0, n - 1);	// Coloca no último
		v.pop_back();		// Remove último
		down(0);			// Desce o primeiro

		return menor;
	}

	else {
		return NULL;
	}
}