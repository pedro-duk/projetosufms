# Gerador de Imagem de Sistema Operacional

## Informações Gerais
* <b> Objetivo: </b> criação de um código em C capaz de, a partir de um bootblock e um kernel fornecidos, gerar uma imagem de Sistema Operacional pronta para ser colocada em um disco bootável. A imagem gerada foi testada usando o emulador Bochs.
* <b> Semestre de realização: </b> Primeiro Semestre de 2022
* <b> Disciplina: </b> Sistemas Operacionais
* <b> Docente: </b> Ronaldo Alves Ferreira

## Como usar
Pré-requisito: [Emulador Bochs](https://bochs.sourceforge.io/getcurrent.html)

1. Inserir arquivo buildimage.c na pasta "Arquivos Dados";
3. Rodar makefile (comando make no terminal);
4. Rodar emulador bochs (comando bochs no terminal);
5. Executar comando "c" no bochs, fazendo com que inicie emulação.

* Obs. 1: "Arquivos dados.zip" contém 3 kernels diferentes para teste. Para testar outros, basta alterar linha 17 do Makefile. Por exemplo, para testar o kernel de tamanho médio, trocar linha por "kernel: kernel-medium.o";
* Obs. 2: Ambos arquivos de bootblock e kernel devem estar no formato ELF.

## Funcionamento

### Compilação
1. É criado um arquivo de imagem vazio;
2. Bootblock é lido, alocando-se espaço suficiente de acordo com a tabela ELF presente no início do arquivo;
3. Bootblock é inserido na imagem no primeiro setor. É feito padding, e os dois bytes finais do setor são definidos como `0x55` e `0xAA`, indicando que é a imagem é bootável;
4. Kernel é lido, alocando-se espaço suficiente de acordo com a tabela ELF presente no início do arquivo. A tabela ELF também indica o número de setores que devem ser alocados para comportar o kernel;
5. Kernel é inserido na imagem, a partir do segundo setor;
6. O número de setores do kernel é inserido como um inteiro a partir do terceiro byte da imagem. Esse número indica ao emulador quantos setores deve ler e carregar na memória.

### Emulação a partir do Bochs
1. Bochs lê a imagem de disco gerada e a carrega na memória;
2. Ao receber o comando "c", bochs começa a executar código do bootblock, que irá carregar o kernel na memória e mudar a execução para código do kernel;
3. Bochs irá executar códigos do kernel, que realizam diferentes testes dependendo do kernel escolhido.

## Limitações
Não foram testados kernels maiores que "kernel-large.s".
