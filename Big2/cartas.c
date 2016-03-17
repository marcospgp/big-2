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
#define DECK		"http://127.0.0.1/big-2/card-images"

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
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%lld_%lld_%d_%d"

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

	long long int hands[4];   // M�os dos 4 jogadores. A primeira dever� ser sempre a do utilizador, de modo a que sej� f�cil averiguar que cartas tem num dado momento
	int cardCount[4];         // N�mero de cartas de cada jogador, na mesma ordem de hands[]
	int consecutivePasses;    // N�mero de passes consecutivos que foram realizados
	long long int selection;  // Cartas selecionadas atualmente pelo utilizador
	long long int lastPlay;   // Cartas na �ltima jogada (sem contar passes. se houveram 3 passes seguidos, lastPlay = 0)
	bool pass, play;          // Se o �tlimo clique do utilizador representa uma a��o

} state;

/** \brief Processa a string recebida como par�metro e retorna o estado atual do jogo

    @param stateString  A string que cont�m a informa��o sobre o atual estado de jgoo
    @return             A informa��o contida na string recebida num formato utiliz�vel em c�digo
*/
state stringToState (char* str) {
	state e;
	sscanf(str, PARAMETER_STRING_FORMAT, &e.hands[0], &e.hands[1], &e.hands[2], &e.hands[3], &e.cardCount[0], &e.cardCount[1], &e.cardCount[2], &e.cardCount[3], &e.consecutivePasses, &e.selection, &e.lastPlay, &e.pass, &e.play);
	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return             Uma string que cont�m toda a informa��o do estado atual do jogo, pronta a ser usada como par�metro
*/
char* stateToString (state e) {
	static char res[10240];
	sprintf(res, PARAMETER_STRING_FORMAT, e.hands[0], e.hands[1], e.hands[2], e.hands[3], e.cardCount[0], e.cardCount[1], e.cardCount[2], e.cardCount[3], e.consecutivePasses, e.selection, e.lastPlay, e.pass, e.play);
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
    @param suit	    O naipe da carta (inteiro entre 0 e 3)
    @param value   	O valor da carta (inteiro entre 0 e 12)
    @return		    O novo estado
*/
long long int addCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A m�o a ser modificada
    @param suit 	O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    O novo estado
*/
long long int removeCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

    @param hand     A m�o a ser verificada
    @param suit	    O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    1 se a carta existe e 0 caso contr�rio
*/
int cardExists (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return (hand >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

    @param path	        O URL correspondente � pasta que cont�m todas as cartas
    @param x            A coordenada x da carta
    @param y            A coordenada y da carta
    @param naipe	    O naipe da carta (inteiro entre 0 e 3)
    @param valor	    O valor da carta (inteiro entre 0 e 12)
    @param gameState    O estado de jogo atual
    @param rotate       Se a carta deve ser imprimida rodada
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState, bool rotate) {

	// Criar um estado que ser� usado se o utilizador clicar nesta carta

	state stateAfterClick = gameState;

	// Se a carta pertence ao utilizador
	bool isUserCard = cardExists(gameState.hands[0], suit, value);

    // Classes html desta carta
	char cardElementClasses[256] = "card";

	// Rota��o da carta
	char cardRotationClass[32] = "card-rotate";

	// Classe que desativa cliques na carta
	char cardDisableClass[32] = "disabled";

	if (!rotate) {

        cardRotationClass[0] = '\0';
	}

    // Se a carta for do utilizador
    if (isUserCard) {

        // Mudar as classes html desta carta (para aplicar estilos personalizados)
        strcpy(cardElementClasses, "card user-card");

        // Se a carta j� est� selecionada
        if (cardExists(gameState.selection, suit, value)) {

            // Ao clicar nela ser� descelecionada
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            // Ao clicar nela ser� selecioanda
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }

        // N�o adicionar � carta a classe que a desativa
        cardDisableClass[0] = '\0';

    } // Else, clicar na carta n�o faz nada

	// Criar url que ser� usado se esta carta for clicada, usando o estado que j� foi criado acima
	char onClickUrl[10240];

	sprintf(onClickUrl, "%s?q=%s", SCRIPT, stateToString(stateAfterClick));

	printf("<a xlink:href = \"%s\"><image class=\"%s %s %s\" x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", onClickUrl, cardElementClasses, cardRotationClass, cardDisableClass, x, y, path, VALUES[value], SUITS[suit]);
}

/** \brief Imprime um estado de jogo

    Esta fun��o imprime o estado atual do jogo no browser

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg width = \"800\" height = \"800\">\n");
    printf("\n<filter id=\"drop-shadow\">\n<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"5\"/>\n<feOffset dx=\"2\" dy=\"2\" result=\"offsetblur\"/>\n<feFlood flood-color=\"rgba(0,0,0,0.5)\"/>\n<feComposite in2=\"offsetblur\" operator=\"in\"/>\n<feMerge>\n<feMergeNode/>\n<feMergeNode in=\"SourceGraphic\"/>\n</feMerge>\n</filter>");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Espa�o entre cartas
	int spaceBetweenCards = 30;

	// Posi��es iniciais para cada m�o
	int hand1x = 185, hand1y = 650;
	int hand2x = 685, hand2y = 520;
	int hand3x = (hand1x + (spaceBetweenCards * 12)), hand3y = 20;
	int hand4x = 35, hand4y = (hand2y - (spaceBetweenCards * 12)); // As duas m�os laterais s�o imprimidas na vertical uma ao contr�rio da outra

	int handx[4] = {hand1x, hand2x, hand3x, hand4x};
	int handy[4] = {hand1y, hand2y, hand3y, hand4y};

    int i, j, k;

    for (j = 0; j < 13; j++) { // Percorrer valores

        for (i = 0; i < 4; i++) { // Percorrer naipes

            for (k = 0; k < 4; k++) { // Percorrer todas as m�os e descobrir se a carta pertence a uma delas

                if (cardExists(gameState.hands[k], i, j)) {

                    if (k == 1 || k == 3) { // Se esta carta pertence a uma m�o lateral, temos de a imprimir rodada

                        // Devido � forma como a carta � rodada, temos de trocar as coordenadas e inverter a coordenada y
                        printCard(path, handx[k], handy[k], i, j, gameState, true);

                        // Incrementar ou diminuir o y para a pr�xima carta nas m�os imprimidas verticalmente
                        if (k == 1) {
                            handy[k] -= 30;
                        } else {
                            handy[k] += 30;
                        }

                    } else {

                        // Se a carta for do utilizador e estiver selecionada, imprime-se mais acima
                        // (bastava verificar se a carta est� selecionada, mas assim se a m�o
                        // a ser imprimida n�o for a do jogador passamos logo � frente)
                        if ((k == 0) && (cardExists(gameState.selection, i, j))) {

                            printCard(path, handx[k], (handy[k] - 20), i, j, gameState, false);

                        } else {

                            printCard(path, handx[k], handy[k], i, j, gameState, false);
                        }

                        if (k == 0) {

                            // Incrementar o x para a pr�xima carta na m�o de baixo
                            handx[k] += 30;

                        } else {

                            // Decrementar o x para a pr�xima carta na m�o de cima
                            handx[k] -= 30;
                        }
                    }
                }
            }
        }
	}

	printf("</svg>\n");

	// Imprimir bot�es

	printf("<div id=\"button-container\">");

	// Bot�o de jogar

	state stateAfterPlay = gameState;

	stateAfterPlay.play = true;

	char playStateString[10240];

	sprintf(playStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPlay));

	printf("<a href=\"%s\" class=\"btn green\">Jogar</a>", playStateString);

	// Bot�o de passar

	state stateAfterPass = gameState;

	stateAfterPass.pass = true;

	char passStateString[10240];

	sprintf(passStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPass));

	printf("<a href=\"%s\" class=\"btn orange\">Passar</a>", passStateString);

	// Bot�o de limpar

	state stateAfterClear = gameState;

	stateAfterClear.selection = 0;

	char clearStateString[10240];

	sprintf(clearStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterClear));

	printf("<a href=\"%s\" class=\"btn purple\">Limpar</a>", clearStateString);

	// Bot�o de recome�ar

	printf("<a href=\"%s\" class=\"btn red\">Recome�ar</a>", SCRIPT);

	printf("</div>");
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

/** \brief Decide se a sele��o atual do jogador � jog�vel

    @param gameState    O estado de jogo atual
    @return             True a sele��o for jog�vel
*/
bool isSelectionPlayable (state gameState) {

    /* Inserir c�digo aqui

        Formato do estado de jogo:

        typedef struct State {

            long long int hands[4];   // M�os dos 4 jogadores. A primeira dever� ser sempre a do utilizador, de modo a que sej� f�cil averiguar que cartas tem num dado momento
            int cardCount[4];         // N�mero de cartas de cada jogador, na mesma ordem de hands[]
            int consecutivePasses;    // N�mero de passes consecutivos que foram realizados
            long long int selection;  // Cartas selecionadas atualmente pelo utilizador
            long long int lastPlay;   // Cartas na �ltima jogada (sem contar passes. se houveram 3 passes seguidos, lastPlay = 0)
            bool pass, play;          // Se o �tlimo clique do utilizador representa uma a��o

        } state;

    */

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

	if(sscanf(query, "q=%s", stateString) == 1) {

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
	printf("<head><title>Exemplo</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../big-2/style.css\">\n</head>\n");
	printf("<body>\n");

    /*
     * Ler os valores passados � cgi que est�o na vari�vel ambiente e pass�-los ao programa
     */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
