# Web Crawler STF

## Informações Gerais
* <b> Objetivo: </b> criação de um programa em Python capaz de baixar acórdãos do STF automaticamente, usando o framework Selenium.
* <b> Semestre: </b> Segundo Semestre de 2020
* <b> Docente: </b> Eraldo Luís Rezende Fernandes (Iniciação Científica)

## Como usar
* Pré-requisitos: 
    - Pacotes gerais: `python3` , `firefox`
    - Pacotes Python3: `lxml`, `bs4`, `selenium`;
1. Escolher os ministros a serem buscados modificando a lista `listaministros`, presente na linha 43 do arquivo `crawlerstf.py`. Por padrão, todos os ministros (`listaministrocompleta`) são incluídos;
2. Escolher as datas inicial e final da busca modificando a linha 46 e 47;
3. Executar o programa pelo comando `python3 crawlerstf.py` (Linux).
