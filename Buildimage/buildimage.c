// Autor: Pedro Luiz da Costa Silva
// Cria uma imagem de Sistema Operacional pronta para ser colocada em um disco bootável

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] <bootblock> <kernel>"

#define SECTOR_SIZE 512       		// Tamanho do setor em bytes
#define BOOTLOADER_SIG_OFFSET 0x1fe // Offset para a assinatura do bootloader (no final do primeiro setor)

// Função para leitura do ELF Header, a partir de fp
void lerElfHeader(Elf32_Ehdr* Ehdr, FILE* fp) {
    fread(&Ehdr->e_ident,       sizeof(unsigned char), EI_NIDENT, fp);      // e_ident
    fread(&Ehdr->e_type,        sizeof(Elf32_Half), 1, fp);                 // e_type
    fread(&Ehdr->e_machine,     sizeof(Elf32_Half), 1, fp);                 // e_machine
    fread(&Ehdr->e_version,     sizeof(Elf32_Word), 1, fp);                 // e_version
    fread(&Ehdr->e_entry,       sizeof(Elf32_Addr), 1, fp);                 // e_entry
    fread(&Ehdr->e_phoff,       sizeof(Elf32_Off), 1, fp);                  // e_phoff
    fread(&Ehdr->e_shoff,       sizeof(Elf32_Off), 1, fp);                  // e_shoff
    fread(&Ehdr->e_flags,       sizeof(Elf32_Word), 1, fp);                 // e_flags
    fread(&Ehdr->e_ehsize,      sizeof(Elf32_Half), 1, fp);                 // e_ehsize
    fread(&Ehdr->e_phentsize,   sizeof(Elf32_Half), 1, fp);                 // e_phentsize
    fread(&Ehdr->e_phnum,       sizeof(Elf32_Half), 1, fp);                 // e_phnum
    fread(&Ehdr->e_shentsize,   sizeof(Elf32_Half), 1, fp);                 // e_shentsize
    fread(&Ehdr->e_shnum,       sizeof(Elf32_Half), 1, fp);                 // e_shnum
    fread(&Ehdr->e_shstrndx,    sizeof(Elf32_Half), 1, fp);                 // e_shstrndx
}

// Função para leitura do Program Header, a partir de fp
void lerProgHeader(Elf32_Phdr* Phdr, FILE* fp) {
    fread(&Phdr->p_type,        sizeof(Elf32_Word), 1, fp);                 // p_type
    fread(&Phdr->p_offset,      sizeof(Elf32_Off), 1, fp);                  // p_offset
    fread(&Phdr->p_vaddr,       sizeof(Elf32_Addr), 1, fp);                 // p_vaddr
    fread(&Phdr->p_paddr,       sizeof(Elf32_Addr), 1, fp);                 // p_paddr
    fread(&Phdr->p_filesz,      sizeof(Elf32_Word), 1, fp);                 // p_filesz
    fread(&Phdr->p_memsz,       sizeof(Elf32_Word), 1, fp);                 // p_memsz
    fread(&Phdr->p_flags,       sizeof(Elf32_Word), 1, fp);                 // p_flags
    fread(&Phdr->p_align,       sizeof(Elf32_Word), 1, fp);                 // p_align
}

// Lê um arquivo executável no formato ELF
Elf32_Phdr * read_exec_file(FILE **execfile, char *filename, Elf32_Ehdr **ehdr)
{ 
	// Abrindo arquivo
    FILE* fp = *execfile;

    // Fazendo leitura do Elf Header
    lerElfHeader(*ehdr, fp);

    // Colocando posição de fp no Program header
    fseek(fp, (long int)(*ehdr)->e_phoff, SEEK_SET);

    // Verifica quantos programas estão no kernel_header.
	int num_prog = (*ehdr)->e_phnum;

    // Alocando espaço para Program Header.
    Elf32_Phdr* Phdr = malloc(num_prog * sizeof(Elf32_Phdr));

    // Fazendo leitura do Program Header, para cada programa.
    for(int i = 0; i < num_prog; i++) {
    	lerProgHeader(&Phdr[i], fp);
    }
    
    // Retornando endereço do espaço alocado
    return Phdr;
}

// Insere bootblock no arquivo de imagem
void write_bootblock(FILE **imagefile,FILE *bootfile,Elf32_Ehdr *boot_header, Elf32_Phdr *boot_phdr)
{
	// Buffer para os dados de bootfile
	unsigned char buffer[boot_phdr->p_memsz + 1];

	// Lendo os dados de bootfile e armazenando no buffer
	fseek(bootfile, boot_phdr->p_offset, SEEK_SET);
	fread(buffer, boot_phdr->p_memsz, 1, bootfile);

	// Colocando posição de imagefile onde o Bootloader deve estar: No primeiro setor da imagem!
	fseek((*imagefile), 0L, SEEK_SET);

	// Escrevendo os dados do Bootloader em imagefile
	fwrite(buffer, boot_phdr->p_memsz, 1, (*imagefile));

	// PADDING: preenchendo até o final do setor atual!
	long int padding = SECTOR_SIZE - ftell((*imagefile)) % SECTOR_SIZE;
	unsigned char charNull = 0x00;
	fwrite(&charNull, sizeof(unsigned char), padding, (*imagefile));
	
	// Colocando posição de imagefile onde o Bootloader Signature deve estar
	fseek((*imagefile), BOOTLOADER_SIG_OFFSET, SEEK_SET);

	// Escrevendo o Bootloader Signature: bytes 0x55 e 0xAA, a partir da posição indicada
	unsigned char byte1 = 0x55;
	unsigned char byte2 = 0xAA;
	fwrite(&byte1, sizeof(unsigned char), 1, (*imagefile));
	fwrite(&byte2, sizeof(unsigned char), 1, (*imagefile));
}

// Insere kernel no arquivo de imagem
void write_kernel(FILE **imagefile,FILE *kernelfile,Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{ 
	// Colocando posição de imagefile onde o Kernel começa: a partir do setor 2.
	fseek((*imagefile), SECTOR_SIZE, SEEK_SET);

	// Verifica quantos programas estão no kernel_header.
	int num_prog = kernel_header->e_phnum;

	// Para cada programa...
	for(int i = 0; i < num_prog; i++) {
		// Buffer para os dados de kernelfile
		unsigned char buffer[kernel_phdr[i].p_memsz + 1];

		// Ler dados de kernelfile e armazenando no buffer
		fseek(kernelfile, kernel_phdr[i].p_offset, SEEK_SET);
		fread(buffer, kernel_phdr[i].p_memsz, 1, kernelfile);

		// Escrever na memória
		fwrite(buffer, kernel_phdr[i].p_memsz, 1, (*imagefile));
	}

	// PADDING: preenchendo até o final do setor atual!
	long int padding = SECTOR_SIZE - ftell((*imagefile)) % SECTOR_SIZE;
	unsigned char charNull = 0x00;
	fwrite(&charNull, sizeof(unsigned char), padding, (*imagefile));
}

// Conta o número de setores no kernel
int count_kernel_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{   
	// Verifica quantos programas estão no kernel_header.
	int num_prog = kernel_header->e_phnum;

	// Soma a memória (p_memsz) de todos os programas, em bytes.
	Elf32_Word soma = 0;

	for(int i = 0; i < num_prog; i++) {
		soma += kernel_phdr[i].p_memsz;
	}

	// Obtém o número de setores, como o quociente entre soma e o tamanho de cada setor.
    int num_sec = soma/SECTOR_SIZE;

    // Coloca mais um setor caso a divisão não tenha sido exata.
    if (kernel_phdr->p_memsz % SECTOR_SIZE != 0) {
    	num_sec++;
    }

    // Retorna o número de setores.
    return num_sec;
}

// Grava o número de setores no kernel
void record_kernel_sectors(FILE **imagefile,Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr, int num_sec)
{
	// Gravar em imagefile, no terceiro byte!
	fseek((*imagefile), 2, SEEK_SET);
	fwrite(&num_sec, sizeof(int), 1, (*imagefile));
}


// Informações do programa conforme o buildimage fornece para a opção --extended
void extended_opt(Elf32_Phdr *bph, int k_phnum, Elf32_Phdr *kph, int num_sec)
{
	int paddingAcumulada = 0;

	// Bootblock
	printf("0x%04x: ./bootblock\n", bph->p_vaddr);
 	printf("\tsegment 0\n");

  	printf("\t\toffset 0x%04x", bph->p_offset);
  	printf("\t\tvaddr 0x%04x", bph->p_vaddr);
  	printf("\n");

  	printf("\t\tfilesz 0x%04x", bph->p_filesz);
  	printf("\t\tmemsz 0x%04x", bph->p_memsz);
  	printf("\n");

  	printf("\t\twriting 0x%04x bytes", bph->p_memsz);
  	printf("\n");

  	paddingAcumulada += bph->p_memsz;

  	if(bph->p_memsz % SECTOR_SIZE != 0) {
  		paddingAcumulada += SECTOR_SIZE - bph->p_memsz % SECTOR_SIZE;
  	}

  	// Kernel
  	printf("\t\tpadding up to 0x%04x", paddingAcumulada);
  	printf("\n");

	printf("0x%04x: ./kernel\n", kph->p_vaddr);
  	for(int i = 0; i < k_phnum; i++) {
  		printf("\tsegment %d\n", i);

  		printf("\t\toffset 0x%04x", kph[i].p_offset);
  		printf("\t\tvaddr 0x%04x", kph[i].p_vaddr);
  		printf("\n");

  		printf("\t\tfilesz 0x%04x", kph[i].p_filesz);
  		printf("\t\tmemsz 0x%04x", kph[i].p_memsz);
  		printf("\n");

  		printf("\t\twriting 0x%04x bytes", kph[i].p_memsz);
  		printf("\n");

  		paddingAcumulada += kph[i].p_memsz;
  	
  		if(kph[i].p_memsz % SECTOR_SIZE != 0) {
  			paddingAcumulada += SECTOR_SIZE - kph[i].p_memsz % SECTOR_SIZE;
  		}

  		printf("\t\tpadding up to 0x%04x", paddingAcumulada);
  		printf("\n");
  	}

  	// Tamanho do SO
	printf("os_size: %d sectors\n", num_sec);
}

// Main
int main(int argc, char **argv)
{
	if(argc < 3 || argc > 4) {
		printf("Uso correto: ");
		printf(ARGS);
		printf("\n");
		return 1;
	}

	int indiceBoot, indiceKernel;

	if(argc == 3) { // só kernel e bootblock
		indiceBoot = 1;
		indiceKernel = 2;
	} else { // --extended, kernel e bootblock
		indiceBoot = 2;
		indiceKernel = 3;
	}

	FILE *kernelfile, *bootfile, *imagefile;  				// ponteiros de arquivo para bootblock, kernel and image
	Elf32_Ehdr *boot_header   = malloc(sizeof(Elf32_Ehdr)); // bootblock ELF header
	Elf32_Ehdr *kernel_header = malloc(sizeof(Elf32_Ehdr)); // kernel ELF header
	
	Elf32_Phdr *boot_program_header;   // bootblock ELF program header
	Elf32_Phdr *kernel_program_header; // kernel ELF program header

	kernelfile = fopen(argv[indiceKernel], "r");
    bootfile = fopen(argv[indiceBoot], "r");
    
    if (kernelfile == NULL || bootfile == NULL || imagefile == NULL) {
        return 1;
    }

	// Criar arquivo de imagem
	imagefile = fopen(IMAGE_FILE, "w");

	// Ler executável: bootblock
	boot_program_header = read_exec_file(&bootfile, argv[indiceBoot], &boot_header);

	// Inserir bootblock na imagem
	write_bootblock(&imagefile, bootfile, boot_header, boot_program_header);

	// Ler executável: kernel
	kernel_program_header = read_exec_file(&kernelfile, argv[indiceKernel], &kernel_header);

	// Inserir segmentos de kernel na imagem
	write_kernel(&imagefile, kernelfile, kernel_header, kernel_program_header);

	// Dizer ao bootloader quantos segmentos ler do kernel
	int num_sec = count_kernel_sectors(kernel_header, kernel_program_header);			// Obtém a quantidade de setores
	record_kernel_sectors(&imagefile, kernel_header, kernel_program_header, num_sec);	// Passa para bootloader

	// Checando por opção --extended
	if(!strncmp(argv[1], "--extended", 11)) {
		extended_opt(boot_program_header, kernel_header->e_phnum, kernel_program_header, num_sec);
	} else {
		printf("0x%04x: ./bootblock\n", boot_program_header->p_vaddr);
		printf("0x%04x: ./kernel\n", kernel_program_header->p_vaddr);
	}

	// Desalocando espaços de memória
	free(boot_header);
	free(boot_program_header);
	free(kernel_header);
	free(kernel_program_header);

	// Fechando arquivos
	fclose(kernelfile);
	fclose(bootfile);
	fclose(imagefile);
  
	return 0;
}

