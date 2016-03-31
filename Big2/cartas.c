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
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d"

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

	long long int hands[4];     /* M�os dos 4 jogadores. A primeira dever� ser sempre a do utilizador, de modo a que sej� f�cil averiguar que cartas tem num dado momento */
	long long int lastPlays[4]; /* As 4 �ltimas jogadas, que ser�o apresentadas na mesa. 0 implica um passe, e todos os bits a 1 implica que aquele jogador ainda n�o fez nada no jogo atual. */
                                /* No sentido anti-hor�rio, lastPlays[0] refere-se � �ltima jogada do utilizador, e lastPlays[1] � �ltima jogada do bot � sua direita */
	long long int selection;    /* Cartas selecionadas atualmente pelo utilizador */
	bool pass, play;            /* Se o �tlimo clique do utilizador representa uma a��o */

} state;

/** \brief Processa a string recebida como par�metro e retorna o estado atual do jogo

    @param stateString  A string que cont�m a informa��o sobre o atual estado de jgoo
    @return             A informa��o contida na string recebida num formato utiliz�vel em c�digo
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
        &e.lastPlays[0],
        &e.lastPlays[1],
        &e.lastPlays[2],
        &e.lastPlays[3],
        &e.selection,
        &e.pass,
        &e.play
    );

	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return             Uma string que cont�m toda a informa��o do estado atual do jogo, pronta a ser usada como par�metro
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
        e.lastPlays[0],
        e.lastPlays[1],
        e.lastPlays[2],
        e.lastPlays[3],
        e.selection,
        e.pass,
        e.play
    );

	return res;
}

/** \brief Devolve o �ndice da carta

    @param suit     O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    O �ndice correspondente � carta
*/
int getCardIndex (int suit, int value) {
	return (value * 4) + suit; /* �ndice da carta reflete o seu poder */
}

/** \brief Devolve o n�mero de cartas numa m�o

    @param hand     A m�o a ser contada
    @return         N�mero de cartas na m�o
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

    @param hand     A m�o a ser modificada
    @param suit	    O naipe da carta (inteiro entre 0 e 3)
    @param value   	O valor da carta (inteiro entre 0 e 12)
    @return		    A m�o modificada
*/
long long int addCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A m�o a ser modificada
    @param suit 	O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    A m�o modificada
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

    @param path	         O URL correspondente � pasta que cont�m todas as cartas
    @param x             A coordenada x da carta
    @param y             A coordenada y da carta
    @param suit	         O naipe da carta (inteiro entre 0 e 3)
    @param value	     O valor da carta (inteiro entre 0 e 12)
    @param gameState     O estado de jogo atual (para determinar urls ap�s cliques nas cartas)
    @param cardPosition  Usado para a rota��o. 0 - cima, 1 - direita, 2 - baixo, 3 - esquerda
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState, int cardPosition) {

	/* Criar um estado que ser� usado se o utilizador clicar nesta carta */

	state stateAfterClick = gameState;

	/* Se a carta pertence ao utilizador */
	bool isUserCard = cardExists(gameState.hands[0], suit, value);

    /* Classes html desta carta */
	char cardElementClasses[256] = "card";

	/* Classe de rota��o da carta */
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

	/* Classe que desativa cliques na carta */
	char cardDisableClass[32] = "disabled";

    /* Se a carta for do utilizador */
    if (isUserCard) {

        /* N�o adicionar � carta a classe que a desativa */
        cardDisableClass[0] = '\0';

        /* Mudar as classes html desta carta (para aplicar estilos personalizados) */
        strcpy(cardElementClasses, "card user-card");

        /* Se a carta j� est� selecionada */
        if (cardExists(gameState.selection, suit, value)) {

            /* Ao clicar nela ser� descelecionada */
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            /* Ao clicar nela ser� selecionada */
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }

    } /* Else, clicar na carta n�o faz nada */

	/* Criar url que ser� usado se esta carta for clicada, usando o estado que j� foi criado acima */
	char onClickUrl[10240];

	sprintf(onClickUrl, "%s?q=%s", SCRIPT, stateToString(stateAfterClick));

	printf("<a href=\"%s\"><img src=\"%s/%c%c.svg\" class=\"%s %s %s\" style=\"position: absolute; left:%dpx; top: %dpx; height: 110px; width: 76px;\"></a>\n", onClickUrl, path, VALUES[value], SUITS[suit], cardElementClasses, cardRotationClass, cardDisableClass, x, y);
}

/** \brief Imprime o html correspondente a um passe

    @param x             A coordenada x de onde deve ser imprimido o html
    @param y             A coordenada y de onde deve ser imprimido o html
*/
void printPass (int x, int y) {

    /* Temos de alterar um bocadinho as coordenadas */
    y += 80;
    x -= 50;

    printf("<span class=\"pass-text\" style=\"position: absolute; left: %d; top: %d;\">Passou</span>\n", x, y);

}

/** \brief Imprime um estado de jogo

    Esta fun��o imprime o estado atual do jogo no browser

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg width = \"800\" height = \"800\">\n");

	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	printf("</svg>\n");

	/* Anotar quem j� jogou para n�o haver confus�o ao imprimir as cartas */
	/* (porque se ainda n�o houveram jogadas, o valor de lastplay ser� ~((long long int) 0))) */

	bool hasPlayed[4] = {true, true, true, true};     /* Quais jogadores j� jogaram */
	bool hasPassed[4] = {false, false, false, false}; /* Quais jogadores passaram */

	int m;
	for (m = 0; m < 4; m++) {

        if (gameState.lastPlays[m] == 0) { /* Se este jogador passou */

            hasPassed[m] = true;

        } else if (~(gameState.lastPlays[m]) == 0) { /* Se este jogador ainda n�o fez nada neste jogo */

            hasPlayed[m] = false;
        }
	}

    /* Largura das cartas (n�o pode ser modificado aqui, read only) */
    int cardWidth = 80;

	/* Espa�o entre cartas */
	int spaceBetweenCards = 30;

	/* Posi��es iniciais para cada m�o */
	/*        m�o 3 */
	/* m�o 4        m�o 2 */
	/*        m�o 1 */
	int hand1x = 180, hand1y = 650;
	int hand2x = 685, hand2y = 520;
	int hand3x = (hand1x + (spaceBetweenCards * 12)), hand3y = 20;
	int hand4x = 35, hand4y = (hand2y - (spaceBetweenCards * 12)); /* As duas m�os laterais s�o imprimidas na vertical uma ao contr�rio da outra */

	int play1x = hand1x + 190, play1y = hand1y - 150;
	int play2x = hand2x - 190, play2y = hand2y - 190;
	int play3x = play1x, play3y = hand3y + 130;
	int play4x = play2x - 270, play4y = play2y;

	int handx[4] = {hand1x, hand2x, hand3x, hand4x};
	int handy[4] = {hand1y, hand2y, hand3y, hand4y};

	int playx[4] = {play1x, play2x, play3x, play4x};
	int playy[4] = {play1y, play2y, play3y, play4y};

	/* Calcular o distanciamento das m�os em pixeis em rela��o � sua posi��o original com base no seu tamanho */

	int handDeltas[4], playDeltas[4];

    int l;
    for (l = 0; l < 4; l++) {

        int handLength = getHandLength(gameState.hands[l]);
        int lastPlayLength = getHandLength(gameState.lastPlays[l]);

        int handLengthPx = cardWidth + ( spaceBetweenCards * ( handLength - 1 ) );
        /* int lastPlayLengthPx = cardWidth + ( spaceBetweenCards * ( lastPlayLength - 1 ) ); */

        /* A desloca��o � de 1/(13 * 2) da largura da m�o por cada carta removida (por cada carta a menos de 13) */
        int deltaHand = (13 - handLength) * ( ( 1 / (26) ) * handLengthPx );

        /* A desloca��o � de 1/2 * spaceBetweenCards por cada carta acima de 1 */

        int deltaLastPlay;

        if (lastPlayLength > 0) {

           deltaLastPlay = (lastPlayLength - 1) * ( (1/2) * spaceBetweenCards);

        } else {

            deltaLastPlay = 0;
        }

        handDeltas[l] = deltaHand;
        playDeltas[l] = deltaLastPlay;
    }

	/* Aplicar deltas �s posi��es originais */

	handx[0] += handDeltas[0];
	handy[1] -= handDeltas[1];
	handx[2] -= handDeltas[2];
	handy[3] += handDeltas[3];

	playx[0] -= playDeltas[0];
	playx[1] -= playDeltas[1];
	playx[2] -= playDeltas[2];
	playx[3] -= playDeltas[3];

    int i, j, k;

    for (j = 0; j < 13; j++) { /* Percorrer valores */

        for (i = 0; i < 4; i++) { /* Percorrer naipes */

            for (k = 0; k < 4; k++) { /* Percorrer todas as m�os / �ltimas jogadas e descobrir se a carta pertence a uma delas */

                if (cardExists(gameState.hands[k], i, j)) {

                    if (k == 0) {

                        /* Se a carta for do utilizador e estiver selecionada, imprime-se mais acima */
                        if (cardExists(gameState.selection, i, j)) {

                            printCard(path, handx[k], (handy[k] - 20), i, j, gameState, 2);

                        } else {

                            printCard(path, handx[k], handy[k], i, j, gameState, 2);
                        }

                        handx[k] += spaceBetweenCards; /* Incrementar o x para a pr�xima carta na m�o de baixo */

                    } else if (k == 1) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 1);

                        handy[k] -= spaceBetweenCards;

                    } else if (k == 2) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 0);

                        handx[k] -= spaceBetweenCards; /* Decrementar o x para a pr�xima carta na m�o de cima */

                    } else if (k == 3) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 3);

                        handy[k] += spaceBetweenCards;
                    }

                } else if (hasPlayed[k] && !hasPassed[k] && cardExists(gameState.lastPlays[k], i, j)) {

                    printCard(path, playx[k], playy[k], i, j, gameState, 2);

                    if (k == 0) {

                        playx[k] += spaceBetweenCards;

                    } else if (k == 1) {

                        playx[k] += spaceBetweenCards;

                    } else if (k == 2) {

                        playx[k] += spaceBetweenCards;

                    } else if (k == 3) {

                        playx[k] += spaceBetweenCards;
                    }
                }
            }
        }
	}

	/* Imprimir os textos "Passou" nos jogadores que passaram nesta jogada */
	int p;
	for (p = 0; p < 4; p++) {

        if (hasPassed[p] == true) {
            printPass(playx[p], playy[p]);
        }
	}

	/* Imprimir bot�es */

	printf("<div id=\"button-container\">");

	/* Bot�o de jogar */

	char playStateString[10240];

	/* Se a sele��o atual for jog�vel */
	if (isSelectionPlayable(gameState)) {

        state stateAfterPlay = gameState;

        stateAfterPlay.play = true;

        sprintf(playStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPlay));

        printf("<a href=\"%s\" class=\"btn green\">Jogar</a>", playStateString);

	} else {

        printf("<a href=\"#\" class=\"btn green disabled\">Jogar</a>");
	}

	/* Bot�o de passar */

	state stateAfterPass = gameState;

	stateAfterPass.pass = true;

	char passStateString[10240];

	sprintf(passStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPass));

	printf("<a href=\"%s\" class=\"btn orange\">Passar</a>", passStateString);

	/* Bot�o de limpar */

	state stateAfterClear = gameState;

	stateAfterClear.selection = 0;

	char clearStateString[10240];

	sprintf(clearStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterClear));

	printf("<a href=\"%s\" class=\"btn purple\">Limpar</a>", clearStateString);

	/* Bot�o de recome�ar */

	printf("<a href=\"%s\" class=\"btn red\">Recome�ar</a>", SCRIPT);

	printf("</div>");
}

/** \brief Distribui cartas por 4 m�os aleatoriamente

    Esta fun��o preenche um array com m�os selecionadas aleatoriamente

    @param hands     Array que vai ser preenchido com as m�os geradas aleatoriamente
*/
void distributeCards (long long int *hands) {

    /* Ter a certeza que as hands est�o a zero */
    int m;
    for (m = 0; m < 4; m++) {
        hands[m] = 0;
    }

    /* Percorrer todos os naipes e cartas e atribu�-las a uma m�o aleat�ria */

    int i, j, handSelected;

    /* char currentSuit, currentValue, currentCardIndex; */

    /* Mant�m a conta de quantas cartas j� foram para cada m�o */
    int cardsInEachHand[4] = {0};

    for (i = 0; i < 4; i++) { /* Percorrer naipes */

            /* currentSuit = SUITS[i]; */

        for (j = 0; j < 13; j++) { /* Percorrer cartas */

            /* currentValue = VALUES[j]; */

            /* currentCardIndex = getCardIndex(i, j); */

            /* Repetir a escolha da m�o at� sair uma que n�o esteja completa */
            do {

                handSelected = rand() % 4;

            } while (cardsInEachHand[handSelected] == 13);

            /* Anotar que esta m�o vai ter mais uma carta */
            cardsInEachHand[handSelected] += 1;

            /* Adicionar a carta � mao selecionada (hands � um array global) */
            hands[handSelected] = addCard(hands[handSelected], i, j);
        }
    }
}

/** \brief Avalia se uma jogada � maior que outra

    N�o � garantido que esta fun��o avalie se ambas as jogadas s�o v�lidas. Essa verifica��o deve ser feita noutro lugar.

    @param play1   A jogada que se quer descobrir se � maior
    @param play2   A jogada contra a qual se quer comparar
    @return        True se a primeira jogada for maior
*/
bool isPlayBigger (long long int play1, long long int play2) {

    /* A primeira coisa a fazer � verificar o tamanho das jogadas */

    int play1len = getHandLength(play1), play2len = getHandLength(play2);

    if (
        play1len == 4 ||
        play1len > 5 ||
        play1len < 1 ||
        play2len == 4 ||
        play2len > 5 ||
        play2len < 1 ||
        play1len != play2len
    ) {
        printf("<!-- Tried to compare two plays when one of them wasn't of valid size. -->");
        return false;
    }

    if (play1len == 1 || play1len == 2 || play1len == 3) {

        /* Quanto maior o poder de uma carta menor o valor do n�mero que a representa
           Nota: N�o estamos a ter em conta se as jogadas s�o v�lidas ou n�o
        */
        return play1 > play2;

    } else {

        /* TODO
           Comparar jogadas de mais que 3 cartas
        */
        printf("<!-- Tried to use undeveloped functionality (play comparison). -->");
        return false;
    }
}

/** \brief Decide se a sele��o atual do jogador � jog�vel

    @param gameState    O estado de jogo atual
    @return             True se a sele��o for jog�vel
*/
bool isSelectionPlayable (state gameState) {

    if (getHandLength(gameState.selection) == 0) {

        return false;

    } else {

        /* Obter a jogada mais recente */

        long long int mostRecentPlay = 0; /* Vai ficar a 0 se n�o for encontrada uma jogada na �ltima ronda */

        int i;
        for (i = 3; i > 0; i--) {

            if (gameState.lastPlays[i] != 0 && ~(gameState.lastPlays[i]) != 0) { /* ~(long long int) 0 significa que aquele jogador ainda n�o fez nada */

                mostRecentPlay = gameState.lastPlays[i];

                break; /* Esta little bitch fez-me perder um pouco de tempo por me ter esquecido de o escrever (>8-/) */
            }
        }

        /* TODO
           Verificar que, se o utilizador tem o 3 de ouros na m�o, tem de o jogar
        */

        int mostRecentPlayLength = getHandLength(mostRecentPlay);
        int selectionLength = getHandLength(gameState.selection);

        /* TODO - Verificar que a sele��o � um par ou um trio v�lido e retornar false caso n�o seja

        if (selectionLength == 2) {

        } else if (selectionLength == 3) {

        } */

        if (mostRecentPlay == 0) { /* Se a jogada mais recente for um passe ou n�o tiver havido jogada mais recente */

           return true;

        } else if (mostRecentPlayLength == selectionLength) { /* Se ambas as m�os t�m o mesmo tamanho */

            /* Retornar true se a sele��o � maior que a jogada do bot que jogou anteriormente */
            return isPlayBigger(gameState.selection, mostRecentPlay);

        } else { /* Se a sele��o n�o for v�lida */

            return false;
        }
    }
}

/** \brief Descobre quem tem o 3 de ouros (e por isso, joga primeiro)

    @param gameState    O estado de jogo atual
    @return             O �ndice da m�o do jogador com o 3 de ouros
*/
int whoGoesFirst (state gameState) {

    int i;
    for (i = 0; i < 4; i++) { /* Percorrer as m�os dos jogadores */

        /* Descobrir quem tem o 3 de ouros */
        if (cardExists(gameState.hands[i], 0, 0)) {

            return i;
        }
    }
}

/** \brief Decide que jogada um bot deve fazer baseando-se no estado de jogo atual

    @param gameState    O estado de jogo atual
    @param index        O �ndice da m�o do bot que est� a jogar (no array hands do estado de jogo)
    @return             Uma m�o que representa as cartas que devem ser jogadas. 0 significa um passe
*/
long long int chooseAIPlay (state gameState, int index) {

    /* Descobrir quais foram as jogadas anteriores */

    long long int lastPlays[3]; /* Array das �ltimas jogadas ordenadas da mais recente para a mais antiga */

    if (index == 1) {

        lastPlays[0] = gameState.lastPlays[0];
        lastPlays[1] = gameState.lastPlays[3];
        lastPlays[2] = gameState.lastPlays[2];

    } else if (index == 2) {

        lastPlays[0] = gameState.lastPlays[1];
        lastPlays[1] = gameState.lastPlays[0];
        lastPlays[2] = gameState.lastPlays[3];

    } else if (index == 3) {

        lastPlays[0] = gameState.lastPlays[2];
        lastPlays[1] = gameState.lastPlays[1];
        lastPlays[2] = gameState.lastPlays[0];

    } else {

        printf("<!-- An AI play was requested with a non-bot index, returned a pass. -->");
        return (long long int) 0;
    }

    /* Descobrir qual foi a jogada v�lida mais recente */

    long long int mostRecentPlay = 0; /* Vai continuar a zero a menos que tenha havido uma jogada v�lida que n�o tenha sido um passe na �ltima ronda */

    int i;
    for (i = 0; i < 3; i++) {

        /* Se:
            - Esta jogada foi uma jogada v�lida ((~(long long int) 0) significa que o jogador ainda n�o fez nada)
            - Esta jogada n�o foi um passe
        */
        if (~(lastPlays[i]) != 0 && lastPlays[i] != 0) {

            /* Anotar o valor da jogada mais recente que tenha sido v�lida e n�o tenha sido um passe */
            mostRecentPlay = lastPlays[i];

            break;
        }
    }

    /* Finalmente, decidir que jogada fazer */

    if (mostRecentPlay == 0) { /* Se n�o houve uma jogada na �ltima ronda */

        /* Jogar a carta mais baixa do baralho */
        int j, k;
        for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
            for (k = 0; k < 4; k++) { /* Percorrer naipes */

                if (cardExists(gameState.hands[index], k, j)) { /* Se o bot que tem de jogar tem esta carta */

                    /* Retornar a carta mais baixa */
                    return addCard((long long int) 0, k, j);
                }
            }
        }

    } else { /* Se houve uma jogada */

        /* Descobrir o n�mero de cartas que temos de jogar */
        int numberOfCardsPlayed = getHandLength(mostRecentPlay);

        if (numberOfCardsPlayed == 1) {

            /* Jogar a carta v�lida mais baixa do baralho */
            int j, k;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        long long int possiblePlay = addCard((long long int) 0, k, j);

                        if (isPlayBigger(possiblePlay, mostRecentPlay)) { /* Se o bot que tem de jogar tem esta carta e � maior que a anterior */

                            /* Retornar a carta v�lida mais baixa */
                            return possiblePlay;
                        }
                    }
                }
            }

            /* N�o encontramos uma jogada v�lida, logo passamos */
            return (long long int) 0;

        } else if (numberOfCardsPlayed == 2) {

            /* Jogar o par v�lido mais baixo do baralho */
            int j, k, l;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se n�o estamos no mesmo naipe da carta pela qual estamos a passar */

                                if (cardExists(gameState.hands[index], l, j)) { /* Se o bot tem esta carta */

                                    long long int possiblePlay = addCard((long long int) 0, k, j);

                                    possiblePlay = addCard(possiblePlay, l, j);

                                    if (isPlayBigger(possiblePlay, mostRecentPlay)) {

                                        return possiblePlay;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /* N�o encontramos uma jogada v�lida, logo passamos */
            return (long long int) 0;

        } else if (numberOfCardsPlayed == 3) {

            /* Jogar o trio v�lido mais baixo do baralho */
            int j, k, l, m;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se n�o estamos no mesmo naipe da carta pela qual estamos a passar */

                                if (cardExists(gameState.hands[index], l, j)) { /* Se o bot tem esta carta */

                                    for (m = 0; m < 4; m++) { /* Tentar encontrar um trio (percorrer naipes) */

                                        if (m != k && m != l) { /* Se n�o estamos no mesmo naipe das cartas do par que encontramos */

                                            if (cardExists(gameState.hands[index], m, j)) { /* Se o bot tem esta carta */

                                                long long int possiblePlay = addCard((long long int) 0, k, j);

                                                possiblePlay = addCard(possiblePlay, l, j);

                                                possiblePlay = addCard(possiblePlay, m, j);

                                                if (isPlayBigger(possiblePlay, mostRecentPlay)) {

                                                    return possiblePlay;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /* N�o encontramos uma jogada v�lida, logo passamos */
            return (long long int) 0;

        } else {

            /* O bot n�o sabe jogar mais que 3 cartas iguais por agora */
            printf("<!-- An AI play was requested of more than 3 cards, and the bot didn't know what to do so it passed. -->");
            return (long long int) 0;
        }
    }
}

/** \brief Processa uma jogada do computador

    Sabendo qual � o bot que tem de jogar, decide que jogada fazer e processa-a

    @param gameState    O estado de jogo atual
    @param index        O �ndice da m�o do bot que est� a jogar (no array hands do estado de jogo)
    @return             O estado de jogo imediatamente ap�s a jogada do computador
*/
state processBotAction (state gameState, int index) {

    /* Decidir que jogada fazer */
    long long int play = chooseAIPlay(gameState, index);

    gameState.lastPlays[index] = play;

    if (play != 0) { /* Se a jogada n�o for um passe (se for um passe n�o � preciso fazer nada) */

        /* Remover da m�o do bot cada carta presente na sua jogada */
        int i, j;
        for (i = 0; i < 4; i++) { /* Percorrer naipes */
            for (j = 0; j < 13; j++) { /* Percorrer valores */

                if (cardExists(play, i, j)) { /* Se a carta fizer parte da jogada */

                    gameState.hands[index] = removeCard(gameState.hands[index], i, j); /* Remov�-la da m�o */
                }
            }
        }
    }

    return gameState;
}

/** \brief Processa uma jogada do utilizador

    Normalmente chamada depois de o utilizador clicar num bot�o (jogar ou passar)

    @param gameState    O estado de jogo atual
    @return             O estado de jogo imediatamente ap�s a a��o
*/
state processUserAction (state gameState) {

    if (gameState.pass) {

        /* Um 0 no array lastPlays implica um passe */
        gameState.lastPlays[0] = 0;

        /* Remover a a��o de passar do estado de jogo */
        gameState.pass = false;

        return gameState; /* Parar a execu��o da fun��o */
    }

    /* Se a a��o n�o foi um passe, ent�o o utilizador clicou no bot�o jogar */

    /* A sele��o j� deve ter sido validada antes de o utilizador carregar no bot�o de jogar, mas aqui fazemos um double check */
    if (!isSelectionPlayable(gameState)) {

        gameState.selection = 0;

        return gameState;
    }

    /* Colocar a jogada mais recente do utilizador no �ndice 0 do array lastPlays (correspondente � �ltima jogada do utilizador) */
    gameState.lastPlays[0] = gameState.selection;

    /* Remover da m�o do jogador cada carta presente na sele��o */
    int i, j;
    for (i = 0; i < 4; i++) { /* Percorrer naipes */
        for (j = 0; j < 13; j++) { /* Percorrer valores */

            if (cardExists(gameState.selection, i, j)) { /* Se a carta estiver selecionada */

                gameState.hands[0] = removeCard(gameState.hands[0], i, j); /* Remov�-la da m�o */
            }
        }
    }

    /* Limpar a sele��o do jogador */
    gameState.selection = 0;

    /* Limpar a a��o jogar do estado de jogo */
    gameState.play = false;

    return gameState;
}

/** \brief Cria um estado de jogo inicial e retorna-o

    Esta fun��o � normalmente usada no in�cio de um jogo para criar um estado inicial

    @return     Um estado de jogo inicial
*/
state getInitialGameState () {

    state e;

    /* Distribuir cartas */
    distributeCards(e.hands);

    /* Todos os bits a 1 numa jogada significa que aquele jogador ainda n�o realizou nenhuma a��o no jogo atual */
    int i;
    for (i = 0; i < 4; i++) {

        e.lastPlays[i] = ~((long long int) 0);
    }

    /* Definir valores iniciais */
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

	if(sscanf(query, "q=%s", stateString) == 1) {

        state gameState = stringToState(stateString);

        if (!gameState.play && !gameState.pass) {

            /* Se h� uma string de par�metros mas o utilizador n�o fez nada, imprimir o estado atual */
            /* (o jogador provavelmente apenas selecionou uma carta) */
            render(gameState);

        } else {

            /* Processar a jogada do utilizador */
            gameState = processUserAction(gameState);

            /* Processar a jogada dos bots */
            int i;
            for (i = 1; i < 4; i++) {

                gameState = processBotAction(gameState, i);
            }

            render(gameState);
        }

	} else {

	    /* Obter um estado de jogo inicial com m�os baralhadas e valores por defeito */
	    state gameState = getInitialGameState();

	    /* Descobrir quem joga primeiro (quem tem o 3 de ouros) */
        int i = whoGoesFirst(gameState);

        /* Processar jogadas dos bots at� ser a vez do utilizador */
        while (i > 0 && i < 4) {

            gameState = processBotAction(gameState, i);

            i++;
        }

		render(gameState);
	}
}

/** \brief Fun��o principal

    Fun��o principal do programa que imprime os cabe�alhos necess�rios e de seguida
    invoca a fun��o que imprime o c�digo html para desenhar as cartas
 */
int main () {

    /* Fornecer uma seed ao rand() */
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
