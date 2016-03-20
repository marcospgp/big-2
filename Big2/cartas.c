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
    Formato da string passada como parâmetro entre jogadas
*/
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%lld_%lld_%d_%d"

/**
    Definir o tipo bool
*/
typedef int bool;
#define true 1
#define false 0

/**
    Estrutura que contém o estado do jogo num dado momento
*/
typedef struct State {

	long long int hands[4];     // Mãos dos 4 jogadores. A primeira deverá ser sempre a do utilizador, de modo a que sejá fácil averiguar que cartas tem num dado momento
	long long int lastPlays[4]; // As 4 últimas jogadas, que serão apresentadas na mesa
	int cardCount[4];           // Número de cartas de cada jogador, na mesma ordem de hands[]
	int consecutivePasses;      // Número de passes consecutivos que foram realizados
	long long int selection;    // Cartas selecionadas atualmente pelo utilizador
	long long int lastPlay;     // Cartas na última jogada (sem contar passes. se houveram 3 passes seguidos, lastPlay = 0)
	bool pass, play;            // Se o útlimo clique do utilizador representa uma ação

} state;

/** \brief Processa a string recebida como parâmetro e retorna o estado atual do jogo

    @param stateString  A string que contém a informação sobre o atual estado de jgoo
    @return             A informação contida na string recebida num formato utilizável em código
*/
state stringToState (char* str) {
	state e;
	sscanf(
        str,
        PARAMETER_STRING_FORMAT,
        &e.hands[0],
        &e.hands[1],
        &e.hands[2],
        &e.hands[3],
        &e.cardCount[0],
        &e.cardCount[1],
        &e.cardCount[2],
        &e.cardCount[3],
        &e.consecutivePasses,
        &e.selection,
        &e.lastPlay,
        &e.pass,
        &e.play
    );

	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return             Uma string que contém toda a informação do estado atual do jogo, pronta a ser usada como parâmetro
*/
char* stateToString (state e) {
	static char res[10240];
	sprintf(
         res,
         PARAMETER_STRING_FORMAT,
         e.hands[0],
         e.hands[1],
         e.hands[2],
         e.hands[3],
         e.cardCount[0],
         e.cardCount[1],
         e.cardCount[2],
         e.cardCount[3],
         e.consecutivePasses,
         e.selection,
         e.lastPlay,
         e.pass,
         e.play
    );

	return res;
}

/** \brief Devolve o índice da carta

    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O índice correspondente à carta
*/
int getCardIndex (int naipe, int valor) {
	return naipe * 13 + valor;
}

/** \brief Devolve o número de cartas numa mão

    @param hand     A mão a ser contada
    @return         Número de cartas na mão
*/
int getHandLength (long long int hand) {

    int cardCount = 0;
    long long int number = hand;

    while (number != 0) {

        if ((number % 2) == 1) {
            cardCount++;
        }

        number /= 2;
    }

    return cardCount;
}

/** \brief Adiciona uma carta ao estado

    @param hand     A mão a ser modificada
    @param suit	    O naipe da carta (inteiro entre 0 e 3)
    @param value   	O valor da carta (inteiro entre 0 e 12)
    @return		    O novo estado
*/
long long int addCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A mão a ser modificada
    @param suit 	O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    O novo estado
*/
long long int removeCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

    @param hand     A mão a ser verificada
    @param suit	    O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    1 se a carta existe e 0 caso contrário
*/
int cardExists (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return (hand >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

    @param path	         O URL correspondente à pasta que contém todas as cartas
    @param x             A coordenada x da carta
    @param y             A coordenada y da carta
    @param suit	         O naipe da carta (inteiro entre 0 e 3)
    @param value	     O valor da carta (inteiro entre 0 e 12)
    @param gameState     O estado de jogo atual
    @param cardPosition  Usado para a rotação. 0 - cima, 1 - direita, 2 - baixo, 3 - esquerda
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState, int cardPosition) {

	// Criar um estado que será usado se o utilizador clicar nesta carta

	state stateAfterClick = gameState;

	// Se a carta pertence ao utilizador
	bool isUserCard = cardExists(gameState.hands[0], suit, value);

    // Classes html desta carta
	char cardElementClasses[256] = "card";

	// Classe de rotação da carta
	char cardRotationClass[32];

	if (cardPosition == 0) {

        strcpy(cardRotationClass, "card-top");

	} else if (cardPosition == 1) {

	    strcpy(cardRotationClass, "card-right");

	} else if (cardPosition == 2) {

	    strcpy(cardRotationClass, "card-bottom");

	} else if (cardPosition == 3) {

	    strcpy(cardRotationClass, "card-left");
	}

	// Classe que desativa cliques na carta
	char cardDisableClass[32] = "disabled";

    // Se a carta for do utilizador
    if (isUserCard) {

        // Não adicionar à carta a classe que a desativa
        cardDisableClass[0] = '\0';

        // Mudar as classes html desta carta (para aplicar estilos personalizados)
        strcpy(cardElementClasses, "card user-card");

        // Se a carta já está selecionada
        if (cardExists(gameState.selection, suit, value)) {

            // Ao clicar nela será descelecionada
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            // Ao clicar nela será selecionada
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }

    } // Else, clicar na carta não faz nada

	// Criar url que será usado se esta carta for clicada, usando o estado que já foi criado acima
	char onClickUrl[10240];

	sprintf(onClickUrl, "%s?q=%s", SCRIPT, stateToString(stateAfterClick));

	printf("<a xlink:href = \"%s\"><image class=\"%s %s %s\" x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", onClickUrl, cardElementClasses, cardRotationClass, cardDisableClass, x, y, path, VALUES[value], SUITS[suit]);
}

/** \brief Imprime um estado de jogo

    Esta função imprime o estado atual do jogo no browser

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg width = \"800\" height = \"800\">\n");
    printf("\n<filter id=\"drop-shadow\">\n<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"5\"/>\n<feOffset dx=\"2\" dy=\"2\" result=\"offsetblur\"/>\n<feFlood flood-color=\"rgba(0,0,0,0.5)\"/>\n<feComposite in2=\"offsetblur\" operator=\"in\"/>\n<feMerge>\n<feMergeNode/>\n<feMergeNode in=\"SourceGraphic\"/>\n</feMerge>\n</filter>");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Espaço entre cartas
	int spaceBetweenCards = 30;

	// Posições iniciais para cada mão
	//        mão 3
	// mão 4        mão 2
	//        mão 1
	int hand1x = 180, hand1y = 650;
	int hand2x = 685, hand2y = 520;
	int hand3x = (hand1x + (spaceBetweenCards * 12)), hand3y = 20;
	int hand4x = 35, hand4y = (hand2y - (spaceBetweenCards * 12)); // As duas mãos laterais são imprimidas na vertical uma ao contrário da outra

	int handx[4] = {hand1x, hand2x, hand3x, hand4x};
	int handy[4] = {hand1y, hand2y, hand3y, hand4y};

    int i, j, k;

    for (j = 0; j < 13; j++) { // Percorrer valores

        for (i = 0; i < 4; i++) { // Percorrer naipes

            for (k = 0; k < 4; k++) { // Percorrer todas as mãos e descobrir se a carta pertence a uma delas

                if (cardExists(gameState.hands[k], i, j)) {

                    if (k == 0) {

                        // Se a carta for do utilizador e estiver selecionada, imprime-se mais acima
                        if (cardExists(gameState.selection, i, j)) {

                            printCard(path, handx[k], (handy[k] - 20), i, j, gameState, 2);

                        } else {

                            printCard(path, handx[k], handy[k], i, j, gameState, 2);
                        }

                        handx[k] += 30; // Incrementar o x para a próxima carta na mão de baixo

                    } else if (k == 1) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 1);

                        handy[k] -= 30;

                    } else if (k == 2) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 0);

                        handx[k] -= 30; // Decrementar o x para a próxima carta na mão de cima

                    } else if (k == 3) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 3);

                        handy[k] += 30;
                    }
                }
            }
        }
	}

	printf("</svg>\n");

	// Imprimir botões

	printf("<div id=\"button-container\">");

	// Botão de jogar

	char playStateString[10240];

	// Se a seleção atual for jogável
	if (isSelectionPlayable(gameState)) {

        state stateAfterPlay = gameState;

        stateAfterPlay.play = true;

        sprintf(playStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPlay));

        printf("<a href=\"%s\" class=\"btn green\">Jogar</a>", playStateString);

	} else {

        printf("<a href=\"#\" class=\"btn green disabled\">Jogar</a>");
	}

	// Botão de passar

	state stateAfterPass = gameState;

	stateAfterPass.pass = true;

	char passStateString[10240];

	sprintf(passStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPass));

	printf("<a href=\"%s\" class=\"btn orange\">Passar</a>", passStateString);

	// Botão de limpar

	state stateAfterClear = gameState;

	stateAfterClear.selection = 0;

	char clearStateString[10240];

	sprintf(clearStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterClear));

	printf("<a href=\"%s\" class=\"btn purple\">Limpar</a>", clearStateString);

	// Botão de recomeçar

	printf("<a href=\"%s\" class=\"btn red\">Recomeçar</a>", SCRIPT);

	printf("</div>");
}

/** \brief Distribui cartas por 4 mãos aleatoriamente

    Esta função preenche um array com mãos selecionadas aleatoriamente

    @param hands     Array que vai ser preenchido com as mãos geradas aleatoriamente
*/
void distributeCards (long long int *hands) {

    // Ter a certeza que as hands estão a zero
    int m;
    for (m = 0; m < 4; m++) {
        hands[m] = 0;
    }

    // Percorrer todos os naipes e cartas e atribuí-las a uma mão aleatória

    int i, j, handSelected;

    // char currentSuit, currentValue, currentCardIndex;

    // Mantém a conta de quantas cartas já foram para cada mão
    int cardsInEachHand[4] = {0};

    for (i = 0; i < 4; i++) { // Percorrer naipes

            // currentSuit = SUITS[i];

        for (j = 0; j < 13; j++) { // Percorrer cartas

            // currentValue = VALUES[j];

            // currentCardIndex = getCardIndex(i, j);

            // Repetir a escolha da mão até sair uma que não esteja completa
            do {

                handSelected = rand() % 4;

            } while (cardsInEachHand[handSelected] == 13);

            // Anotar que esta mão vai ter mais uma carta
            cardsInEachHand[handSelected] += 1;

            // Adicionar a carta à mao selecionada (hands é um array global)
            hands[handSelected] = addCard(hands[handSelected], i, j);
        }
    }
}

/** \brief Decide se a seleção atual do jogador é jogável

    @param gameState    O estado de jogo atual
    @return             True se a seleção for jogável
*/
bool isSelectionPlayable (state gameState) {

    /* Inserir código aqui

        Formato do estado de jogo:

        typedef struct State {

            long long int hands[4];   // Mãos dos 4 jogadores. A primeira deverá ser sempre a do utilizador, de modo a que sejá fácil averiguar que cartas tem num dado momento
            int cardCount[4];         // Número de cartas de cada jogador, na mesma ordem de hands[]
            int consecutivePasses;    // Número de passes consecutivos que foram realizados
            long long int selection;  // Cartas selecionadas atualmente pelo utilizador
            long long int lastPlay;   // Cartas na última jogada (sem contar passes. se houveram 3 passes seguidos, lastPlay = 0)
            bool pass, play;          // Se o útlimo clique do utilizador representa uma ação

        } state;

    */

    if (getHandLength(gameState.selection) == 0) {

        return false;

    } else {

        return true;
    }

}

/** \brief Cria um estado de jogo inicial e retorna-o

    Esta função é normalmente usada no início de um jogo para criar um estado inicial

    @return     Um estado de jogo inicial
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

    Esta função recebe a query que é passada à cgi-bin e trata-a.

    @param query A query que é passada à cgi-bin
 */
void parse (char *query) {

	char stateString[1024];

	// const long long int ESTADO_INICIAL = 0xfffffffffffff;

	if(sscanf(query, "q=%s", stateString) == 1) {

        state gameState = stringToState(stateString);

        /*if (gameState.userTurn)

            render(gameState);

            state newGameState = gameState;

        if (newGameState.pass) {


        }*/

		render(gameState);

	} else {

		render(getInitialGameState());
	}
}

/** \brief Função principal

    Função principal do programa que imprime os cabeçalhos necessários e de seguida
    invoca a função que imprime o código html para desenhar as cartas
 */
int main () {

    // Fornecer uma seed ao rand()
    srand(time(NULL));

    /*
     * Cabeçalhos necessários numa CGI
     */
	printf("Content-Type: text/html; charset=iso-8859-1\n\n");
	printf("<head><title>Exemplo</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../big-2/style.css\">\n</head>\n");
	printf("<body>\n");

    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
