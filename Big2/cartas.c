#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
URL da CGI
*/
#define SCRIPT		"http://127.0.0.1/cgi-bin/big2"
/**
URL da pasta com as cartas
*/
#define BARALHO		"http://127.0.0.1/card-images"

/**
Ordem dos naipes
*/
#define NAIPES		"DCHS"
/**
Ordem das cartas
*/
#define VALORES		"3456789TJQKA2"

/**
	Estado inicial com todas as 52 cartas do baralho
	Cada carta � representada por um bit que est�
	a 1 caso ela perten�a � m�o ou 0 caso contr�rio
*/
const long long int ESTADO_INICIAL = 0xfffffffffffff;


/** \brief Devolve o �ndice da carta

@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O �ndice correspondente � carta
*/
int indice(int naipe, int valor) {
	return naipe * 13 + valor;
}

/** \brief Adiciona uma carta ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int add_carta(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int rem_carta(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		1 se a carta existe e 0 caso contr�rio
*/
int carta_existe(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return (ESTADO >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

@param path	o URL correspondente � pasta que cont�m todas as cartas
@param x A coordenada x da carta
@param y A coordenada y da carta
@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
*/
void imprime_carta(char *path, int x, int y, long long int ESTADO, int naipe, int valor) {
	char *suit = NAIPES;
	char *rank = VALORES;
	char script[10240];
	sprintf(script, "%s?q=%lld", SCRIPT, rem_carta(ESTADO, naipe, valor));
	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[valor], suit[naipe]);
}

/** \brief Imprime o estado

Esta fun��o est� a imprimir o estado em quatro colunas: uma para cada naipe
@param path	o URL correspondente � pasta que cont�m todas as cartas
@param ESTADO	O estado atual
*/
void imprime(char *path, long long int ESTADO) {
	int n, v;
	int x, y;

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Nestes for loops, x e y referem-se �s coordenadas onde vai ser imprimida a pr�xima carta
	for(y = 10, n = 0; n < 4; n++, y += 120) {
		for(x = 10, v = 0; v < 13; v++)
			if(carta_existe(ESTADO, n, v)) {
				x += 20;
				imprime_carta(path, x, y, ESTADO, n, v);
			}
	}
	printf("</svg>\n");
}



/** \brief Trata os argumentos da CGI

Esta fun��o recebe a query que � passada � cgi-bin e trata-a.
Neste momento, a query cont�m o estado que � um inteiro que representa um conjunto de cartas.
Cada carta corresponde a um bit que est� a 1 se essa carta est� no conjunto e a 0 caso contr�rio.
Caso n�o seja passado nada � cgi-bin, ela assume que todas as cartas est�o presentes.
@param query A query que � passada � cgi-bin
 */
void parse(char *query) {

	long long int ESTADO;

	if(sscanf(query, "q=%lld", &ESTADO) == 1) {
		imprime(BARALHO, ESTADO);
	} else {
		imprime(BARALHO, ESTADO_INICIAL);
	}
}

/** \brief Fun��o principal

Fun��o principal do programa que imprime os cabe�alhos necess�rios e depois disso invoca
a fun��o que vai imprimir o c�digo html para desenhar as cartas
 */
int main() {
/*
 * Cabe�alhos necess�rios numa CGI
 */
	printf("Content-Type: text/html; charset=utf-8\n\n");
	printf("<head><title>Exemplo</title></head>\n");
	printf("<body>\n");

	printf("<h1>Exemplo de utiliza��o</h1>\n");

/*
 * Ler os valores passados � cgi que est�o na vari�vel ambiente e pass�-los ao programa
 */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
