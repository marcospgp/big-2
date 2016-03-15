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

	long long int hands[4];  // M�os dos 4 jogadores. A primeira dever� ser sempre a do utilizador, de modo a que sej� f�cil averiguar que cartas tem num dado momento
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
	sscanf(str, PARAMETER_STRING_FORMAT, &e.hands[0], &e.hands[1], &e.hands[2], &e.hands[3], &e.cardCount[0], &e.cardCount[1], &e.cardCount[2], &e.cardCount[3], &e.selection, &e.pass, &e.play);
	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return     Uma string que cont�m toda a informa��o do estado atual do jogo, pronta a ser usada como par�metro
*/
char* stateToString (state e) {
	static char res[10240];
	sprintf(res, PARAMETER_STRING_FORMAT, e.hands[0], e.hands[1], e.hands[2], e.hands[3], e.cardCount[0], e.cardCount[1], e.cardCount[2], e.cardCount[3], e.selection, e.pass, e.play);
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

    @param hand     A m�o a ser modificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int addCard (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A m�o a ser modificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int removeCard (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return hand & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

    @param hand     A m�o a ser verificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		1 se a carta existe e 0 caso contr�rio
*/
int cardExists (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return (hand >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

    @param path	        O URL correspondente � pasta que cont�m todas as cartas
    @param x            A coordenada x da carta
    @param y            A coordenada y da carta
    @param naipe	    O naipe da carta (inteiro entre 0 e 3)
    @param valor	    O valor da carta (inteiro entre 0 e 12)
    @param gameState    O estado de jogo atual
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState) {

	// Criar um estado que ser� usado se o utilizador clicar nesta carta

	state stateAfterClick = gameState;

    // Se a carta for do utilizador
    if (cardExists(gameState.hands[0], suit, value)) {

        // Se a carta j� est� selecionada
        if (cardExists(gameState.selection, suit, value)) {

            // Ao clicar nela ser� descelecionada
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            // Ao clicar nela ser� selecioanda
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }
    } // Else, clicar na carta n�o faz nada

	// Criar url que ser� usado se esta carta for clicada, usando o estado que j� foi criado acima
	char onClickUrl[10240];

	sprintf(onClickUrl, "%s?q=%s", SCRIPT, stateToString(stateAfterClick));

	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", onClickUrl, x, y, path, VALUES[value], SUITS[suit]);
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

	for (y = 50, i = 0; i < 4; i++, y += 120) { // Percorrer m�os pelo eixo y

        // Percorrer todas as cartas e imprimir as que estiverem na m�o

        x = 10; // Coordenada x inicial

		for (j = 0; j < 4; j++) { // Percorrer naipes

            for (k = 0; k < 13; k++) { // Percorrer valores

                if (cardExists(gameState.hands[i], j, k)) { // Se a carta estiver na m�o

                    x += 30;

                    // Se a carta for do utilizador e estiver selecionada
                    // (bastava verificar se a carta est� selecionada, mas assim se a m�o a ser imprimida
                    // n�o for a do jogador passamos logo � frente)
                    if ((i == 0) && (cardExists(gameState.selection, j, k))) {

                        printCard(path, x, (y - 20), j, k, gameState);

                    } else {

                        printCard(path, x, y, j, k, gameState);
                    }
                }
            }
        }
	}
	printf("</svg>\n");
}

/** \brief Distribui cartas por 4 m�os aleatoriamente

    Esta fun��o preenche um array com m�os selecionadas aleatoriamente

    @param hands     Array que vai ser preenchido com as m�os geradas aleatoriamente
*/
void distributeCards (long long int *hands) {

    // Ter a certeza que as hands est�o a zero
    int m;
    for (m = 0; m < 4; m++) {
        hands[m] = 0;
    }

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

    state e;

    distributeCards(e.hands);

    // Tem-se de usar um for loop para se reinicializar um array
    int i;
    for (i = 0; i < 4; i++) {
        e.cardCount[i] = 13; // Atribuir uma contagem de 13 cartas a cada jogador
    }

    e.selection = 0;
    e.pass = false;
    e.play = false;

    return e;
}

/** \brief Trata os argumentos da CGI

    Esta fun��o recebe a query que � passada � cgi-bin e trata-a.

    @param query A query que � passada � cgi-bin
 */
void parse (char *query) {

	char stateString[1024];

	// const long long int ESTADO_INICIAL = 0xfffffffffffff;

	if(sscanf(query, "q=%s", &stateString) == 1) {

        state gameState = stringToState(stateString);

		render(gameState);

	} else {

		render(getInitialGameState());
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
