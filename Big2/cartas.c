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
#define DECK		"http://127.0.0.1/card-images"

/**
    Ordem dos naipes
*/
#define SUITS		"DCHS"

/**
    Ordem das cartas
*/
#define VALUES		"3456789TJQKA2"

/**
    Formato da string passada como par�metro entre jogadas
*/
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%d_%d_%d_%d_%lld_%d_%d"

/**
    Definir o tipo bool
*/
typedef int bool;
#define true 1
#define false 0

/**
    Estrutura que cont�m o estado do jogo num dado momento
*/
typedef struct State {

	long long int hands[4];  // M�os dos 4 jogadores
	int cardCount[4];        // N�mero de cartas de cada jogador, na mesma ordem de hands[]
	long long int selection; // Cartas selecionadas atualmente pelo utilizador
	bool pass, play;         // Se o �tlimo clique do utilizador representa uma a��o

} state;

/** \brief Processa a string recebida como par�metro e retorna o estado atual do jogo

    @param stateString  A string que cont�m a informa��o sobre o atual estado de jgoo
    @return     A informa��o contida na string recebida num formato utiliz�vel em c�digo
*/
state stringToState (char* str) {
	state e;
	sscanf(str, PARAMETER_STRING_FORMAT, &e.hand[0], &e.hand[1], &e.hand[2], &e.hand[3], &e.cardCount[0], &e.cardCount[1], &e.cardCount[2], &e.cardCount[3], &e.selection, &e.pass, &e.play);
	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return     Uma string que cont�m toda a informa��o do estado atual do jogo, pronta a ser usada como par�metro
*/
char* stateToString (state gameState) {
	static char res[10240];
	sprintf(res, PARAMETER_STRING_FORMAT, e.hand[0], e.hand[1], e.hand[2], e.hand[3], e.cardCount[0], e.cardCount[1], e.cardCount[2], e.cardCount[3], e.selection, e.pass, e.play);
	return res;
}

/** \brief Devolve o �ndice da carta

    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O �ndice correspondente � carta
*/
int getCardIndex (int naipe, int valor) {
	return naipe * 13 + valor;
}

/** \brief Adiciona uma carta ao estado

    @param ESTADO	O estado atual
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int addCard (long long int ESTADO, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param ESTADO	O estado atual
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int removeCard (long long int ESTADO, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

    @param ESTADO	O estado atual
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		1 se a carta existe e 0 caso contr�rio
*/
int cardExists (long long int ESTADO, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
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
void printCard (char *path, int x, int y, long long int *estados, int naipe, int valor) {
	char *suit = SUITS;
	char *rank = VALUES;
	char script[10240];
	sprintf(script, "%s?q1=%lld&q2=%lldq3=%lld&q4=%lld", SCRIPT, estados[0], estados[1], estados[2], estados[3]);
	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[valor], suit[naipe]);
}

/** \brief Imprime um estado de jogo

    Esta fun��o imprime o estado atual do jogo

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Nestes for loops, x e y referem-se �s coordenadas onde vai ser imprimida a pr�xima carta

	int i, j, k, x, y;

	for (y = 10, i = 0; i < 4; i++, y += 120) { // Percorrer m�os pelo eixo y

        // Percorrer todas as cartas e imprimir as que estiverem na m�o

        x = 10; // Coordenada x inicial

		for (j = 0; j < 4; j++) {

            for (k = 0; k < 13; k++) {

                if (cardExists(estados[i], j, k)) {

                    x += 20;
                    printCard(path, x, y, estados, j, k);

                }
            }
        }
	}
	printf("</svg>\n");
}

/** \brief Distribui cartas pelas 4 m�os aleatoriamente

    Esta fun��o preenche as m�os de um estado de jogo com cartas selecionadas aleatoriamente
*/
void distributeCards (state gameState) {




    // TODO





    // Percorrer todos os naipes e cartas e atribu�-las a uma m�o aleat�ria

    int i, j, handSelected;

    // char currentSuit, currentValue, currentCardIndex;

    // Mant�m a conta de quantas cartas j� foram para cada m�o
    int cardsInEachHand[4] = {0};

    for (i = 0; i < 4; i++) { // Percorrer naipes

            // currentSuit = SUITS[i];

        for (j = 0; j < 13; j++) { // Percorrer cartas

            // currentValue = VALUES[j];

            // currentCardIndex = getCardIndex(i, j);

            // Repetir a escolha da m�o at� sair uma que n�o esteja completa
            do {

                handSelected = rand() % 4;

            } while (cardsInEachHand[handSelected] == 13);

            // Anotar que esta m�o vai ter mais uma carta
            cardsInEachHand[handSelected] += 1;

            // Adicionar a carta � mao selecionada (hands � um array global)
            hands[handSelected] = addCard(hands[handSelected], i, j);
        }
    }
}

/** \brief Cria um estado de jogo inicial e retorna-o

    Esta fun��o � normalmente usada no in�cio de um jogo para criar um estado inicial
*/
state getInitialGameState () {

    // TODO

}

/** \brief Trata os argumentos da CGI

    Esta fun��o recebe a query que � passada � cgi-bin e trata-a.

    @param query A query que � passada � cgi-bin
 */
void parse (char *query) {

	char state[1024];

	// const long long int ESTADO_INICIAL = 0xfffffffffffff;

	if(sscanf(query, "q=%s", &state) == 1) {

        state gameState = stringToState(state);

		render(gameState);

	} else {

		imprime(getInitialGameState());
	}
}

/** \brief Fun��o principal

    Fun��o principal do programa que imprime os cabe�alhos necess�rios e de seguida
    invoca a fun��o que imprime o c�digo html para desenhar as cartas
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
