
# Projeto Buildimage
Este trabalho tem como objetivo a criação de um código em C capaz de, a partir de um bootblock e um kernel fornecidos, gerar uma imagem de Sistema Operacional pronta para ser colocada em um disco bootável. A imagem gerada foi testada usando o emulador Bochs.

# Como testar
Pré-requisito: [Emulador Bochs](https://bochs.sourceforge.io/getcurrent.html)

1. Extrair "Arquivos dados.zip";
2. Inserir arquivo buildimage.c na pasta extraída;
3. Rodar makefile (comando make no terminal);
4. Rodar emulador bochs (comando bochs no terminal);
5. Executar comando "c" no emulador bochs, fazendo com que inicie emulação.

Obs.: "Arquivos dados.zip" contém 3 kernels diferentes para teste. Para testar outros, basta alterar linha 17 do Makefile. Por exemplo, para testar o kernel de tamanho médio, trocar linha por "kernel: kernel-medium.o".
