# Compilador da Linguagem X++

## Informações Gerais
* <b> Objetivo: </b> criação de um programa em C++ capaz de realizar análise léxica, sintática e semântica para a linguagem hipotética X++ descrita no livro “Como Construir um Compilador Utilizando Ferramentas Java” de Márcio Eduardo Delamaro.
* <b> Semestre de realização: </b> Primeiro Semestre de 2022
* <b> Disciplina: </b> Compiladores 1
* <b> Docente: </b> Bianca de Almeida Dantas

## Como usar
1. Compilar arquivos através do comando `g++ \*.cpp -o xpp\_compiler`;
2. Criar arquivos texto para a entrada. Existem arquivos prontos no diretório "testes";
3. Executar o programa gerado através do comando `./xpp_compiler <arqinput>`. Substituir `<arqinput>` pelo caminho do arquivo de entrada a ser testado.

## Funcionamento
* O programa irá percorrer o arquivo de entrada e realizar análise léxica, sintática e semântica seguindo os tokens e gramática da linguagem hipotética X++;
* Caso haja erros na compilação, irá avisar o tipo do erro e apontar onde ocorreu;
* Se o erro for léxico ou sintático, o programa é encerrado;
* Se o erro for semântico, o programa continua até o final do arquivo de entrada, avisando que a compilação foi encerrada com erros;
* Caso não haja erros, o programa indica que a compilação foi encerrada com sucesso.

## Limitações
* Não é um compilador completo, pois não gera código executável;
* Double-free em alguns erros léxicos;
* Análise semântica está incompleta:
	- Não verifica se uma variável é um array;
	- Não verifica range de arrays;
	- Não verifica tipos internos de atributos de classes. Por exemplo:

~~~C++
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
