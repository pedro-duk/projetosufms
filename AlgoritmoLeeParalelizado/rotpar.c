// ----------------------------------------------------------------------------
// Roteamento usando algoritmo de Lee - IMPLEMENTAÇÃO PARALELA
// Trabalho 1 - Programação Paralela - T01 - Nahri Balesdent Moreano - 2022.2
// 
// Feito pelos alunos:
//	- Pedro Luiz da Costa Silva
//	- Maria Elisa Rodrigues Rabello
//
// Para compilar: gcc rotpar.c -fopenmp -o rotpar -Wall
// Para executar: ./rotpar <nome arquivo entrada> <nome arquivo saída>
//
// Observações:
//	- Transformamos a fila (que era encadeada) em uma fila de array.
//		- Fizemos isso para melhor usarmos o construtor for.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <omp.h>

// ----------------------------------------------------------------------------
// Tipos

// Célula: linha e coluna
typedef struct {
	int i, j;
} t_celula;

// Nó da fila de células a serem tratadas e do caminho encontrado 
typedef struct no {				
	int i, j;
	struct no *prox;
} t_no;

// Struct pra filas de células
typedef struct {
	t_celula *ini_fila;
	int num_elementos;
	int tamanho;
} t_fila;

// ----------------------------------------------------------------------------
// Variáveis globais

int n_linhas,				// Número de linhas do grid;
	n_colunas,				// Número de colunas do grid;
	**dist,					// Matriz com distância da origem até cada célula do grid;
	distancia_min;			// Distância do caminho mínimo de origem a destino;

t_celula origem,			// Célula origem no grid;
	destino;				// Célula destino no grid;

t_no *ini_caminho;			// Ponteiro para início do caminho encontrado;

t_fila fila_tratadas;		// Fila de células a serem tratadas.

// ----------------------------------------------------------------------------
// Inicializa uma fila
void inicializa_fila(t_fila* fila) {
	int tam_inicial = 100;

	fila->ini_fila = malloc(sizeof(t_celula) * tam_inicial);

	// Checa se conseguiu alocar
	if (fila->ini_fila == NULL) {
		printf("\nFalha na alocação de memória.\n");
		exit(1);
	}

	fila->num_elementos = 0;
	fila->tamanho = tam_inicial;
}

// ----------------------------------------------------------------------------
// Deleta uma fila
void deleta_fila(t_fila* fila) {
	free(fila->ini_fila);

	fila->num_elementos = 0;
	fila->tamanho = 0;
}

// ----------------------------------------------------------------------------
// Remove elementos da fila (não deleta da memória)
void limpa_fila(t_fila* fila) {
	fila->num_elementos = 0;
}

// ----------------------------------------------------------------------------
// Aumenta tamanho da fila na quantidade especificada
void aumenta_fila(t_fila* fila, int tamanho_novo) {
	// Checa se tamanho não é menor ou igual a quantidade já existente
	if (tamanho_novo <= fila->tamanho) {
		printf("\nFalha na alocação de memória. Tamanho_novo inválido.\n");
		exit(1);
	}

	// Alocar novo espaço
	t_celula* novo_alocado = malloc(sizeof(t_celula) * tamanho_novo);

	// Checa se conseguiu alocar
	if (novo_alocado == NULL) {
		printf("\nFalha na alocação de memória.\n");
		exit(1);
	}

	// Copia elementos de fila para o novo espaço
	memcpy(novo_alocado, fila->ini_fila, sizeof(t_celula) * fila->num_elementos);

	// Desaloca espaço anterior
	free(fila->ini_fila);

	// Muda ponteiros
	fila->ini_fila = novo_alocado;

	// Atualiza tamanho
	fila->tamanho = tamanho_novo;
}

// ----------------------------------------------------------------------------
// Insere célula no fim da fila

void insere_fila(t_fila* fila, t_celula celula) {
	// Garantindo que fila_principal tem espaço suficiente
	if(fila->tamanho == fila->num_elementos) {
		aumenta_fila(fila, (fila->tamanho) * 10);
	}

	// Inserindo no final
	fila->ini_fila[fila->num_elementos] = celula;

	// Incrementando número de elementos
	fila->num_elementos = fila->num_elementos + 1;
}

// ----------------------------------------------------------------------------
// Insere elementos de uma fila em outra fila
void mescla_filas(t_fila* fila_principal, t_fila* fila_inserida) {
	int qtd1 = fila_principal->num_elementos;
	int qtd2 = fila_inserida->num_elementos;

	// Garantindo que fila_principal tem espaço suficiente
	if(fila_principal->tamanho < qtd1 + qtd2) {
		aumenta_fila(fila_principal, (qtd1 + qtd2) * 10);
	}

	// Copia elementos para o novo espaço
	memcpy(fila_principal->ini_fila + qtd1, fila_inserida->ini_fila, sizeof(t_celula) * qtd2);

	// Atualiza quantidade de elementos da fila principal
	fila_principal->num_elementos = qtd1 + qtd2;
}

// ----------------------------------------------------------------------------
// Inicialização global

void inicializa(char* nome_arq_entrada) {
	FILE *arq_entrada;		// Arquivo texto de entrada
	int n_obstaculos,		// Número de obstáculos do grid
		n_linhas_obst,		// Obstáculos são retângulos. Aqui é o número de linhas que o obstáculo tem.
		n_colunas_obst;		// Número de colunas do obstáculo.

	t_celula obstaculo;		// Usado para guardar posição da célula superior esquerda do obstáculo.

	arq_entrada = fopen(nome_arq_entrada, "rt");

	if (arq_entrada == NULL) {
		printf("\nArquivo texto de entrada não encontrado\n");
		exit(1);
	}

	// Recebe tamanho do grid, posição da origem e do destino, e o número de obstáculos.
	fscanf(arq_entrada, "%d %d", &n_linhas, &n_colunas);
	fscanf(arq_entrada, "%d %d", &origem.i, &origem.j);
	fscanf(arq_entrada, "%d %d", &destino.i, &destino.j);
	fscanf(arq_entrada, "%d", &n_obstaculos);

	// Aloca grid
	dist = malloc(n_linhas * sizeof (int*));
	for (int i = 0; i < n_linhas; i++) {
		dist[i] = malloc(n_colunas * sizeof (int));

		// Checa se conseguiu alocar
		if (dist[i] == NULL) {
			printf("\nFalha na alocação de memória.\n");
			exit(1);
		}
	}
	
	// 1- Inicializa todas células com INT_MAX
	// 2- Define distância da origem com 0
	// 3- Define obstáculos com -1

	// Inicializa grid
	for (int i = 0; i < n_linhas; i++) {
		for (int j = 0; j < n_colunas; j++) {
			dist[i][j] = INT_MAX;
		}
	}

	dist[origem.i][origem.j] = 0; // Distância da origem até ela mesma é 0

	// Lê obstáculos do arquivo de entrada e preenche grid
	for (int k = 0; k < n_obstaculos; k++) {
		fscanf(arq_entrada, "%d %d %d %d", &obstaculo.i, &obstaculo.j, &n_linhas_obst, &n_colunas_obst);

		for (int i = obstaculo.i; i < obstaculo.i + n_linhas_obst; i++) {
			for (int j = obstaculo.j; j < obstaculo.j + n_colunas_obst; j++) {
				dist[i][j] = -1;
			}
		}
	}

	// Fecha arquivo de entrada
	fclose(arq_entrada);

	// Inicializa fila vazia
	inicializa_fila(&fila_tratadas);

	// Inicializa caminho vazio
	ini_caminho = NULL;
}

// ----------------------------------------------------------------------------
// Finalização global

void finaliza(char* nome_arq_saida) {
	FILE *arq_saida;	// Arquivo texto de saída
	t_no *no;

	arq_saida = fopen(nome_arq_saida, "wt");

	// Imprime distância mínima no arquivo de saída
	fprintf(arq_saida, "%d\n", distancia_min);

	// Imprime caminho mínimo no arquivo de saída
	while (ini_caminho != NULL) {
		fprintf(arq_saida, "%d %d\n", ini_caminho->i, ini_caminho->j);

		no = ini_caminho;
		ini_caminho = ini_caminho->prox;

		// Libera nó do caminho
		free(no);
	}

	// Fechando arquivo de saída
	fclose(arq_saida);

	// Libera grid
	for (int i = 0; i < n_linhas; i++){
		free(dist[i]);
	}
	free(dist);

	// Deletando filas globais
	deleta_fila(&fila_tratadas);
}

// ----------------------------------------------------------------------------
// Insere célula no inicio do caminho

void insere_caminho(t_celula celula) {
	t_no *no = malloc(sizeof(t_no));
	
	// Checa se conseguiu alocar
	if (no == NULL) {
		printf("\nFalha na alocação de memória.\n");
		exit(1);
	}

	no->i = celula.i;
	no->j = celula.j;
	no->prox = ini_caminho;

	ini_caminho = no;
}

// ----------------------------------------------------------------------------
// Fase de expansão
//	- Aqui é onde exploramos o paralelismo!

bool expansao() {
	// Insere célula origem na fila de células a serem tratadas
	insere_fila(&fila_tratadas, origem);

	// Variáveis compartilhadas entre as threads
	bool achou = false;					// Indica se destino foi encontrado.
	int camada = 1;						// Número atual da camada. Indica distância.
	int qtd_remover;					// Auxiliar que guarda a quantidade atual de células a serem removidas.

	// Início da região paralela!
	#pragma omp parallel
	{
		// Variáveis privadas de cada thread
		t_fila fila_thread;					// Fila individual da thread.
		t_celula vizinho;					// Posição de possíveis vizinhos.

		// Inicializando fila_thread
		inicializa_fila(&fila_thread);

		// Enquanto fila não está vazia e não chegou na célula destino
		while (fila_tratadas.num_elementos > 0 && !achou) {

			// Apenas uma thread executa, outras esperam terminar
			#pragma omp single
			{
				// Guardar quantidade atual de células a serem removidas
				qtd_remover = fila_tratadas.num_elementos;
			} // Barreira implícita

			#pragma omp for
			for(int index_celula = 0; index_celula < qtd_remover; index_celula++) {
				t_celula celula = (fila_tratadas.ini_fila)[index_celula];
				// Checa se chegou ao destino
				if (celula.i == destino.i && celula.j == destino.j) {
					achou = true;
				}

				else {
					// Para cada um dos 4 possíveis vizinhos da célula (norte, sul, oeste e leste):
					// se célula vizinha existe e ainda não possui valor de distância,
					// calcula distância e insere vizinho na fila de células a serem tratadas

					// Sobre o paralelismo, podemos ver que existe dependência de saída:
					//	- Várias threads escrevendo em dist[vizinho.i][vizinho.j].
					// 	- Porém, sempre escrevem o mesmo valor. Então, o valor escrito sempre será correto.
					//	- Um problema que pode acontecer é, para o mesmo vizinho:
					//		- Thread 1 obtém TRUE para "dist = INT_MAX", e portanto entra no if;
					//		- Thread 2 obtém TRUE para "dist = INT_MAX", e também entra no if (antes de 1 atualizar dist);
					//		- Thread 1 atualiza dist e insere na fila;
					//		- Thread 2 atualiza dist e insere na fila.
					//
					//	Portanto teriam duas instâncias da mesma célula na fila global. Isso, porém, não altera o resultado
					// 		final, pois processar a mesma célula duas vezes não muda as distâncias calculadas, que dependem
					//		somente da camada atual. Tentamos, com locks, impedir que células duplicadas fossem inseridas
					//		na fila global, porém o overhead de criação de destruição dos locks fez não valer a pena. Portanto,
					//		as threads às vezes inserem células duplicadas.

					// Vizinho norte
					vizinho.i = celula.i - 1;
					vizinho.j = celula.j;

					if (vizinho.i >= 0) {
						if (dist[vizinho.i][vizinho.j] == INT_MAX) {
							dist[vizinho.i][vizinho.j] = camada;
							insere_fila(&fila_thread, vizinho);
						}
					}

					// Vizinho sul
					vizinho.i = celula.i + 1; 
					vizinho.j = celula.j;

					if (vizinho.i < n_linhas) {
						if (dist[vizinho.i][vizinho.j] == INT_MAX) {
							dist[vizinho.i][vizinho.j] = camada;
							insere_fila(&fila_thread, vizinho);
						}
					}

					// Vizinho oeste
					vizinho.i = celula.i; 
					vizinho.j = celula.j - 1;

					if (vizinho.j >= 0) {
						if (dist[vizinho.i][vizinho.j] == INT_MAX) {
							dist[vizinho.i][vizinho.j] = camada;
							insere_fila(&fila_thread, vizinho);
						}
					}

					// Vizinho leste
					vizinho.i = celula.i; 
					vizinho.j = celula.j + 1;

					if (vizinho.j < n_colunas) {
						if (dist[vizinho.i][vizinho.j] == INT_MAX) {
							dist[vizinho.i][vizinho.j] = camada;
							insere_fila(&fila_thread, vizinho);
						}
					}
				}
			} // Barreira implícita

			// Apenas uma thread executa, outras esperam terminar
			#pragma omp single
			{
				// Limpa fila de células a serem tratadas
				limpa_fila(&fila_tratadas);

				// Aumenta distância para a próxima iteração
				camada++;
			} // Barreira implícita

			// Juntando resultados individuais das threads, só uma thread faz isso por vez!
			#pragma omp critical
			{
				// Inserindo as células na fila global
				mescla_filas(&fila_tratadas, &fila_thread);
			}

			// Limpando a fila individual da thread (não deleta da memória)
			limpa_fila(&fila_thread);

			// Impede que outras threads sigam até todas estarem prontas
			#pragma omp barrier
		}

		// Deletando filas individuais de threads
		deleta_fila(&fila_thread);
	}
	return achou;
}

// ----------------------------------------------------------------------------

void traceback()
{
	t_celula celula, vizinho;

	// Constrói caminho mínimo, com células do destino até a origem
	
	// Inicia caminho com célula destino
	insere_caminho(destino);

	celula.i = destino.i;
	celula.j = destino.j;

	// Enquanto não chegou na origem
	while (celula.i != origem.i || celula.j != origem.j)
	{
		// Determina se célula anterior no caminho é vizinho norte, sul, oeste ou leste
		// e insere esse vizinho no início do caminho
		
		vizinho.i = celula.i - 1; // Norte
		vizinho.j = celula.j;

		if ((vizinho.i >= 0) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
			insere_caminho(vizinho);
		else
		{
			vizinho.i = celula.i + 1; // Sul
			vizinho.j = celula.j;

			if ((vizinho.i < n_linhas) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
				insere_caminho(vizinho);
			else
			{
				vizinho.i = celula.i; // Oeste
				vizinho.j = celula.j - 1;

				if ((vizinho.j >= 0) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
					insere_caminho(vizinho);
				else
				{
					vizinho.i = celula.i; // Leste
					vizinho.j = celula.j + 1;

					if ((vizinho.j < n_colunas) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
						insere_caminho(vizinho);
				}
			}
		}
		celula.i = vizinho.i;
		celula.j = vizinho.j;
	}
}

// ----------------------------------------------------------------------------
// Programa principal

int main(int argc, char** argv) {
	char nome_arq_entrada[100],
		  nome_arq_saida[100];
	bool achou;
	double tempo_ini,
			tempo_fim,
			tempo_ini_expansao,
			tempo_fim_expansao;

	// tempo_ini: No início da execução do programa
	tempo_ini = omp_get_wtime();

	// Checagem de argumentos
	if(argc != 3) {
		printf("O programa foi executado com argumentos incorretos.\n");
		printf("Uso: ./rotpar <nome arquivo entrada> <nome arquivo saída>\n");
		exit(1);
	}

	// Obtém nome dos arquivos de entrada e saída
	strcpy(nome_arq_entrada, argv[1]);
	strcpy(nome_arq_saida, argv[2]);

	//printf("Inicializando estruturas de dados.......");

	// Lê arquivo de entrada e inicializa estruturas de dados
	inicializa(nome_arq_entrada);

	//printf("DONE!\n");
	//printf("Executando fase de expansão.............");

	// tempo_ini_expansao: Logo antes de expansão começar
	tempo_ini_expansao = omp_get_wtime();

	// Fase de expansão: calcula distância da origem até demais células do grid
	achou = expansao();

	// tempo_fim_expansao: Logo depois de expansão acabar
	tempo_fim_expansao = omp_get_wtime();

	//printf("DONE!\n");
	//printf("Executando fase de traceback............");

	// Se não encontrou caminho de origem até destino
	if (!achou) {
		distancia_min = -1;
	}
	else {
		// Obtém distância do caminho mínimo da origem até destino
		distancia_min = dist[destino.i][destino.j];

		// Fase de traceback: obtém caminho mínimo
		traceback();
	}

	//printf("DONE!\n");
	//printf("Finalizando.............................");

	// Finaliza e escreve arquivo de saida
	finaliza(nome_arq_saida);

	// tempo_fim_expansao: Final de execução do programa
	tempo_fim = omp_get_wtime();

	//printf("DONE!\n\n");

	// Escreve tempo de expansão
	printf("Tempo de execução (total): %f\n", tempo_fim - tempo_ini);

	// Escreve tempo de expansão
	printf("Tempo de execução (somente expansão): %f\n", tempo_fim_expansao - tempo_ini_expansao);
	
	return 0;
}