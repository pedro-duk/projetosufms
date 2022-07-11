import requests
import time
import re
import json
import os
import sys
import shutil
import traceback

from lxml import html
from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import TimeoutException
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException


sitebase = 'https://jurisprudencia.stf.jus.br/pages/search?base=acordaos&sinonimo=true&plural=true&pageSize=10&sort=_score&sortBy=desc'

str_pathdocumentos = "docsBaixados"
str_patherros = "erros.jsonl"
str_pathmeta = "metadadosgerais.jsonl"

### Seleção dos critérios de busca ###

# Lista dos ministros a serem buscados
listaministroscompleta = [
    'CELSO DE MELLO',
    'MARCO AURÉLIO',
    'GILMAR MENDES',
    'RICARDO LEWANDOWSKI',
    'CÁRMEN LÚCIA',
    'DIAS TOFFOLI',
    'LUIZ FUX',
    'ROSA WEBER',
    'ROBERTO BARROSO',
    'EDSON FACHIN',
    'ALEXANDRE DE MORAES'
]

listaministros = listaministroscompleta

# Datas inicial e final
str_datainicial = '01022019'
str_datafinal = '31022019'

# Variável para indicar o uso do parâmetro datainicial e final
usardatas = True

### Variáveis globais ###
timestampinicial = 0
timestampfinal = 0
numprocessos = 0
numprocessoserro = 0
listaprocessoserro = []

# Criando diretório para salvar baixados
try:
    os.mkdir(str_pathdocumentos)
except OSError:
    pass

### Funções ###


def carregarPagina(browser, url, selectorcss, numpagina):
    print("\t- Carregando página " + str(numpagina) + "... -")
    try:
        browser.get(url)
        WebDriverWait(browser, 10).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, selectorcss)))
    except TimeoutException:
        print("Página " + url + " demorou para carregar, tentando novamente...")
        carregarPagina(browser, url, selectorcss, numpagina)


def finalizaCrawler():
    print("Crawler finalizado, salvando metadados... ", end='', flush=True)

    # Fechando todas janelas possíveis
    handles = browser.window_handles

    for i in range(0, len(handles)):
        browser.switch_to.window(handles[i])
        browser.close()

    # Obtendo timestampfinal
    timestampfinal = time.time()

    ### Salvando metadados do crawling ###

    metcrawling = {}

    metcrawling["datainicial"] = str_datainicial
    metcrawling["datafinal"] = str_datafinal
    metcrawling["usardatas"] = usardatas
    metcrawling["listaministros"] = listaministros
    metcrawling["timestamp_inicio"] = timestampinicial
    metcrawling["timestamp_final"] = timestampfinal
    metcrawling["numprocessos"] = numprocessos
    metcrawling["numprocessoserro"] = numprocessoserro
    metcrawling["listaprocessoserro"] = listaprocessoserro

    with open(str_pathmeta, "a+", encoding="utf-8") as file:
        file.write(json.dumps(metcrawling))
        file.write('\n')

    print("OK")
    sys.exit()


### Obtendo todos arquivos já baixados, exclui arquivos com erros (sem metadados) ###

listadocsbaixados = set()

print("Obtendo códigos de arquivos baixados...")

pathministros = [
    f.path for f in os.scandir(str_pathdocumentos) if f.is_dir()]

for pastaministro in pathministros:
    print('\tProcurando em ' + pastaministro)

    for f in os.scandir(pastaministro):
        if f.is_dir():
            try:
                with open(f.path + "\\metadados.json", "r", encoding="utf-8") as file:
                    listadocsbaixados.add(json.loads(file.read())['codigo'])
            except FileNotFoundError:
                print("\t\tDeletando arquivo em " + f.path)
                shutil.rmtree(f.path)

print("Obtidos " + str(len(listadocsbaixados)) + " documentos.\n")

### Código ###

# Impressão inicial
print("Iniciando busca no STF, parâmetros:")
print("\tdatainicial:", str_datainicial)
print("\tdatafinal:", str_datafinal)
print("\tusardatas:", usardatas)
print("\tlistaministros:", listaministros)

# Obtendo timestamp inicial
timestampinicial = time.time()

# Acessando url

for i, ministro in enumerate(listaministros):
    # Fazendo pasta para o ministro
    try:
        os.mkdir(str_pathdocumentos + '/' + ministro)
    except OSError:
        pass

    strministro = "&ministro_facet=" + ministro + "&queryString=" + ministro
    strdata = "&&publicacao_data=" + str_datainicial + "-" + str_datafinal

    # Caso usardatas seja true, será incluída na busca
    if usardatas:
        urlbase = sitebase + strministro + strdata
    else:
        urlbase = sitebase + strministro

    url = urlbase + "&page=1"

    # Abrir janela caso seja primeiro elemento
    if i == 0:
        print("Abrindo webdriver firefox...")
        browser = webdriver.Firefox()
        print("Webdriver aberto com sucesso.")

    browser.get(url)

    # Obtendo número total de resultados
    try:
        pathaux = '/html/body/app-root/app-home/main/search/div/div/div/div[2]/div/div[1]/div[1]/span'
        elemento = WebDriverWait(browser, 10).until(
            EC.presence_of_element_located((By.XPATH, pathaux)))
        numtotal = elemento.text.split()[0].replace('.', '')

        if numtotal == 'Nenhum':
            print("Nenhum resultado encontrado! Fechando crawler.")
            sys.exit()

        numtotal = int(numtotal)
        numprocessos += numtotal

    except TimeoutException:
        print("Demorou muito para abrir! Finalizando programa.")
        sys.exit()

    print("--- Baixando " + ministro + " ---")

    # Obtendo número de páginas
    numpaginas = numtotal//10 + 1

    if numtotal % 10 == 0:
        numpaginas -= 1

    # Variável para guardar quantidade de resultados ainda não processados
    restantes = numtotal

    print("\t" + str(numtotal) + " acórdãos encontrados em", numpaginas, "páginas.")

    for pag in range(1, numpaginas + 1):
        # Calculando qtdpagina e restantes
        qtdpagina = 10

        if restantes < 10:
            qtdpagina = restantes
            restantes = 0

        else:
            restantes -= 10

        # Abrindo página. Caso seja a primeira, não precisa reabrir
        if pag != 1:
            urlprox = urlbase + "&page=" + str(pag)

            # Esperando carregar...
            pathaux = '#result-index-0 > div:nth-child(2) > div:nth-child(1) > a:nth-child(4) > mat-icon:nth-child(1)'
            carregarPagina(browser, urlprox, pathaux, pag)

        # Iniciando a busca e download dos arquivos
        print("\tBaixando " + str(qtdpagina) + " arquivos na página " +
              str(pag) + "/" + str(numpaginas) + "...")

        for i in range(0, qtdpagina):
            try:
                # Achando código e verificando se já foi baixado
                aux = '#result-index-' + \
                    str(i) + '> a:nth-child(1) > h4:nth-child(1)'

                codigo = browser.find_element(By.CSS_SELECTOR, aux).text
                if codigo in listadocsbaixados:
                    print("\t\tAcórdão " + codigo + " já baixado.")
                    continue

                # Variável para guardar metadados
                dictmetadados = {}

                # Metadados: Código do processo
                dictmetadados['codigo'] = codigo

                # Obtendo link e título
                aux = '#result-index-' + \
                    str(i) + ' > div:nth-child(2) > div:nth-child(1) > a:nth-child(4)'

                browser.find_element(By.CSS_SELECTOR, aux).click()
                browser.switch_to.window(browser.window_handles[1])

                pathaux = '#mainContainer'
                WebDriverWait(browser, 4).until(
                    EC.presence_of_element_located((By.CSS_SELECTOR, pathaux)))

                link = browser.current_url

                titulo = link[link.find('docID'):].replace(
                    "docID", "idDocumento")

                browser.close()
                browser.switch_to.window(browser.window_handles[0])

                # Criando pastas
                str_pathatual = str_pathdocumentos + '/' + ministro + '/' + titulo

                try:
                    os.mkdir(str_pathatual)
                except OSError:
                    print(
                        "\t%s já existente." % str_pathatual)
                    continue

                # Salvando metadados
                resultado = browser.find_element(By.CSS_SELECTOR, 
                    '#result-index-' + str(i))

                # Metadados: Header
                resheader = resultado.find_element(By.ID,
                    'result-principal-header').text.split('\n')

                for elem in resheader:
                    elemsplit = elem.split(': ')
                    dictmetadados[elemsplit[0]] = elemsplit[1]

                # Metadados: Resto. Nem sempre possui isso.
                try:
                    resultoutros = resultado.find_element(By.ID,
                        'other-occurrences')

                    listaoutros = resultoutros.find_elements(By.XPATH,"./*")

                    for elem in listaoutros:
                        if elem.text != '':
                            # Pegamos a ementa aqui!
                            aux = elem.text.split('\n', 1)

                            if len(aux) > 1:
                                dictmetadados[aux[0]] = aux[1]

                        else:
                            aux = re.sub(
                                ' +', ' ', elem.get_attribute('textContent').strip())

                            aux = re.sub('arrow_drop_up', '', aux)

                            aux2 = aux.split()[0]

                            if 'Indexação' in aux2:
                                dictmetadados['Indexação'] = re.sub(
                                    'Indexação', '', aux)
                            elif 'Legislação' in aux2:
                                dictmetadados['Legislação'] = re.sub(
                                    'Legislação', '', aux)
                            elif 'Partes' in aux2:
                                dictmetadados['Partes'] = re.sub(
                                    'Partes', '', aux)
                            elif 'Acórdãos no mesmo sentido' in aux2:
                                dictmetadados['Acórdãos no mesmo sentido'] = re.sub(
                                    'Acórdãos no mesmo sentido', '', aux)

                except NoSuchElementException:
                    # Devemos pegar a ementa!
                    resultado.find_elements(By.CLASS_NAME,"mb-15")
                    auxxx = resultado.text

                # Baixando documento
                print("\tBaixando arquivo " + titulo +
                      "... ", end='', flush=True)

                try:
                    r = requests.get(link)

                    with open(str_pathatual + "/" + titulo + ".pdf", "wb") as file:
                        file.write(r.content)

                    # Adicionando o código se deu pra baixar!
                    listadocsbaixados.add(codigo)

                except OSError:
                    print("Arquivo já existente!")
                    continue

                # Criando arquivo dos metadados
                with open(str_pathatual + "/metadados.json", "w", encoding="utf-8") as file:
                    file.write(json.dumps(dictmetadados))

                print("OK")
            except Exception as e:
                print("\t---Exceção no documento número " + str(i) + "---")
                listaprocessoserro.append(dictmetadados)
                numprocessoserro += 1

                dictmetadados['ministro'] = ministro
                dictmetadados['paginas'] = str(pag)
                dictmetadados['numero'] = str(i)
                dictmetadados["datainicial"] = str_datainicial
                dictmetadados["datafinal"] = str_datafinal

                with open(str_patherros, 'a+') as outfile:
                    json.dump(dictmetadados, outfile)
                    outfile.write('\n')

                # Fechando outras possíveis janelas abertas
                handles = browser.window_handles

                for i in range(1, len(handles)):
                    browser.switch_to.window(handles[i])
                    browser.close()

                browser.switch_to.window(handles[0])

finalizaCrawler()
