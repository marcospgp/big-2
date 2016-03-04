#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
    URL da CGI
*/
#define SCRIPT		"http://127.0.0.1/cgi-bin/big2.exe"

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
    Definir o tipo bool
*/
typedef int bool;
#define true 1
#define false 0

/**
	Estado inicial com todas as 52 cartas do baralho
	Cada carta � representada por um bit que est�
	a 1 caso ela perten�a � m�o ou 0 caso contr�rio
*/
const long long int ESTADO_INICIAL = 0xfffffffffffff;

/**
    Array global que cont�m as 4 m�os
*/
static long long int hands[4];

/** \brief Devolve o �ndice da carta

@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O �ndice correspondente � carta
*/
int indice (int naipe, int valor) {
	return naipe * 13 + valor;
}

/** \brief Adiciona uma carta ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int add_carta (long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int rem_carta (long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		1 se a carta existe e 0 caso contr�rio
*/
int carta_existe (long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return (ESTADO >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

@param path	o URL correspondente � pasta que cont�m todas as cartas
@param x A coordenada x da carta
@param y A coordenada y da carta
@param estados	As 4 m�os atuais
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
*/
void imprime_carta (char *path, int x, int y, long long int *estados, int naipe, int valor) {
	char *suit = NAIPES;
	char *rank = VALORES;
	char script[10240];
	sprintf(script, "%s?q1=%lld&q2=%lldq3=%lld&q4=%lld", SCRIPT, estados[0], estados[1], estados[2], estados[3]);
	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[valor], suit[naipe]);
}

/** \brief Imprime o estado

Esta fun��o est� a imprimir o estado em quatro colunas: uma para cada naipe

@param firstPlay	true se esta for a primeira jogada
@param estados      As 4 m�os caso esta n�o seja a primeira jogada (pointer para um array). Este param�tro n�o deve existir se o primeiro argumento for true
*/
void imprime (bool firstPlay, long long int *estados) {

	int i, j, k;
	int x, y;

	char *path = BARALHO;

	if (firstPlay) {

        distributeCards(); // Preenche o array global hands

        estados = hands;
	}

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Nestes for loops, x e y referem-se �s coordenadas onde vai ser imprimida a pr�xima carta

	for (y = 10, i = 0; i < 4; i++, y += 120) { // Percorrer m�os pelo eixo y

        // Percorrer todas as cartas e imprimir as que estiverem na m�o

        x = 10; // Coordenada x inicial

		for (j = 0; j < 4; j++) {

            for (k = 0; k < 13; k++) {

                if (carta_existe(estados[i], j, k)) {

                    x += 20;
                    imprime_carta(path, x, y, estados, j, k);

                }
            }
        }
	}
	printf("</svg>\n");
}

/** \brief Distribui cartas pelas 4 m�os aleatoriamente

    Esta fun��o preenche o array global que cont�m as m�os com cartas aleat�rias,
    e deve ser normalmente chamada no in�cio de cada jogo.
*/
void distributeCards () {

    // Percorrer todos os naipes e cartas e atribu�-las a uma m�o aleat�ria

    int i, j, handSelected;

    // char currentSuit, currentValue, currentCardIndex;

    // Mant�m a conta de quantas cartas j� foram para cada m�o
    int cardsInEachHand[4] = {0};

    int z;
    for (z = 0; z < 4; z++) {
        printf("<!-- %d -->", cardsInEachHand[z]);
    }

    for (i = 0; i < 4; i++) { // Percorrer naipes

            // currentSuit = NAIPES[i];

        for (j = 0; j < 13; j++) { // Percorrer cartas

            // currentValue = VALORES[j];

            // currentCardIndex = indice(i, j);

            // Repetir a escolha da m�o at� sair uma que n�o esteja completa
            do {

                handSelected = rand() % 4;

            } while (cardsInEachHand[handSelected] == 13);

            // Anotar que esta m�o vai ter mais uma carta
            cardsInEachHand[handSelected] += 1;

            // Adicionar a carta � mao selecionada (hands � um array global)
            hands[handSelected] = add_carta(hands[handSelected], i, j);
        }
    }
}


/** \brief Trata os argumentos da CGI

Esta fun��o recebe a query que � passada � cgi-bin e trata-a.
Neste momento, a query cont�m o estado que � um inteiro que representa um conjunto de cartas.
Cada carta corresponde a um bit que est� a 1 se essa carta est� no conjunto e a 0 caso contr�rio.
Caso n�o seja passado nada � cgi-bin, ela assume que todas as cartas est�o presentes.
@param query A query que � passada � cgi-bin
 */
void parse (char *query) {

	long long int estado1, estado2, estado3, estado4;

	if(sscanf(query, "q1=%lld&q2=%lld&q3=%lld&q4=%lld", &estado1, &estado2, &estado3, &estado4) == 4) {

		long long int estados[] = {estado1, estado2, estado3, estado4};

		imprime(false, estados);

	} else {

		imprime(true, NULL);
	}
}

/** \brief Fun��o principal

Fun��o principal do programa que imprime os cabe�alhos necess�rios e depois disso invoca
a fun��o que vai imprimir o c�digo html para desenhar as cartas
 */
int main () {

    // Fornecer uma seed ao rand()
    srand(time(NULL));

/*
 * Cabe�alhos necess�rios numa CGI
 */
	printf("Content-Type: text/html; charset=iso-8859-1\n\n");
	printf("<head><title>Exemplo</title></head>\n");
	printf("<body style=\"background-color: #007700; color: #ffffff;\">\n");

	printf("<h1>Exemplo de utiliza��o</h1>\n");

/*
 * Ler os valores passados � cgi que est�o na vari�vel ambiente e pass�-los ao programa
 */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
