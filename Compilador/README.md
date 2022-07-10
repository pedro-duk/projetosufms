# Projeto Compilador

## Informações Gerais
* <b> Objetivo </b>: criação de um programa em C++ capaz de realizar análise léxica, sintática e semântica para a linguagem hipotética X++ descrita no livro “Como Construir um Compilador Utilizando Ferramentas Java” de Márcio Eduardo Delamaro.
* <b> Semestre de realização</b>: 2022.1

## Objetivo

Este trabalho tem como objetivo a 

## Limitações
* Double-free em alguns erros léxicos;
* Análise semântica está incompleta:
	- Não verifica se uma variável é um array;
	- Não verifica range de arrays;
	- Em atribuições com acessos a atributos internos de classes, não verifica se os atributos das classes existem. O tipo de cada lado é igual ao tipo da primeira variável. Por exemplo:

~~~c
class Classe1 {
    int num;
}

class Classe2 {
    int num;
}

class Main {
    int main() {
        Classe1 c1;
        Classe2 c2;
		
        c1.num = +c2.num;
    }
}
~~~

Nessa atribuição, o compilador acredita que o tipo da esquerda é "Classe1", e o da direita é "Classe2". Portanto, gerará um erro semântico de tipos incompatíveis (apesar de não haver esse erro).
