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
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d"

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

	long long int hands[4];     /* Mãos dos 4 jogadores. A primeira deverá ser sempre a do utilizador, de modo a que sejá fácil averiguar que cartas tem num dado momento */
	long long int lastPlays[4]; /* As 4 últimas jogadas, que serão apresentadas na mesa. 0 implica um passe, e todos os bits a 1 implica que aquele jogador ainda não fez nada no jogo atual. */
                                /* No sentido anti-horário, lastPlays[0] refere-se à última jogada do utilizador, e lastPlays[1] à última jogada do bot à sua direita */
	long long int selection;    /* Cartas selecionadas atualmente pelo utilizador */
	bool pass, play;            /* Se o útlimo clique do utilizador representa uma ação */

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

/** \brief Devolve o índice da carta

    @param suit     O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    O índice correspondente à carta
*/
int getCardIndex (int suit, int value) {
	return (value * 4) + suit; /* Índice da carta reflete o seu poder */
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
    @return		    A mão modificada
*/
long long int addCard (long long int hand, int suit, int value) {
	int idx = getCardIndex(suit, value);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A mão a ser modificada
    @param suit 	O naipe da carta (inteiro entre 0 e 3)
    @param value	O valor da carta (inteiro entre 0 e 12)
    @return		    A mão modificada
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
    @param gameState     O estado de jogo atual (para determinar urls após cliques nas cartas)
    @param cardPosition  Usado para a rotação. 0 - cima, 1 - direita, 2 - baixo, 3 - esquerda
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState, int cardPosition) {

	/* Criar um estado que será usado se o utilizador clicar nesta carta */

	state stateAfterClick = gameState;

	/* Se a carta pertence ao utilizador */
	bool isUserCard = cardExists(gameState.hands[0], suit, value);

    /* Classes html desta carta */
	char cardElementClasses[256] = "card";

	/* Classe de rotação da carta */
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

        /* Não adicionar à carta a classe que a desativa */
        cardDisableClass[0] = '\0';

        /* Mudar as classes html desta carta (para aplicar estilos personalizados) */
        strcpy(cardElementClasses, "card user-card");

        /* Se a carta já está selecionada */
        if (cardExists(gameState.selection, suit, value)) {

            /* Ao clicar nela será descelecionada */
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            /* Ao clicar nela será selecionada */
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }

    } /* Else, clicar na carta não faz nada */

	/* Criar url que será usado se esta carta for clicada, usando o estado que já foi criado acima */
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

    Esta função imprime o estado atual do jogo no browser

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg width = \"800\" height = \"800\">\n");

	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	printf("</svg>\n");

	/* Anotar quem já jogou para não haver confusão ao imprimir as cartas */
	/* (porque se ainda não houveram jogadas, o valor de lastplay será ~((long long int) 0))) */

	bool hasPlayed[4] = {true, true, true, true};     /* Quais jogadores já jogaram */
	bool hasPassed[4] = {false, false, false, false}; /* Quais jogadores passaram */

	int m;
	for (m = 0; m < 4; m++) {

        if (gameState.lastPlays[m] == 0) { /* Se este jogador passou */

            hasPassed[m] = true;

        } else if (~(gameState.lastPlays[m]) == 0) { /* Se este jogador ainda não fez nada neste jogo */

            hasPlayed[m] = false;
        }
	}

    /* Largura das cartas (não pode ser modificado aqui, read only) */
    int cardWidth = 80;

	/* Espaço entre cartas */
	int spaceBetweenCards = 30;

	/* Posições iniciais para cada mão */
	/*        mão 3 */
	/* mão 4        mão 2 */
	/*        mão 1 */
	int hand1x = 180, hand1y = 650;
	int hand2x = 685, hand2y = 520;
	int hand3x = (hand1x + (spaceBetweenCards * 12)), hand3y = 20;
	int hand4x = 35, hand4y = (hand2y - (spaceBetweenCards * 12)); /* As duas mãos laterais são imprimidas na vertical uma ao contrário da outra */

	int play1x = hand1x + 190, play1y = hand1y - 150;
	int play2x = hand2x - 190, play2y = hand2y - 190;
	int play3x = play1x, play3y = hand3y + 130;
	int play4x = play2x - 270, play4y = play2y;

	int handx[4] = {hand1x, hand2x, hand3x, hand4x};
	int handy[4] = {hand1y, hand2y, hand3y, hand4y};

	int playx[4] = {play1x, play2x, play3x, play4x};
	int playy[4] = {play1y, play2y, play3y, play4y};

	/* Calcular o distanciamento das mãos em pixeis em relação à sua posição original com base no seu tamanho */

	int handDeltas[4], playDeltas[4];

    int l;
    for (l = 0; l < 4; l++) {

        int handLength = getHandLength(gameState.hands[l]);
        int lastPlayLength = getHandLength(gameState.lastPlays[l]);

        int handLengthPx = cardWidth + ( spaceBetweenCards * ( handLength - 1 ) );
        /* int lastPlayLengthPx = cardWidth + ( spaceBetweenCards * ( lastPlayLength - 1 ) ); */

        /* A deslocação é de 1/(13 * 2) da largura da mão por cada carta removida (por cada carta a menos de 13) */
        int deltaHand = (13 - handLength) * ( ( 1 / (26) ) * handLengthPx );

        /* A deslocação é de 1/2 * spaceBetweenCards por cada carta acima de 1 */

        int deltaLastPlay;

        if (lastPlayLength > 0) {

           deltaLastPlay = (lastPlayLength - 1) * ( (1/2) * spaceBetweenCards);

        } else {

            deltaLastPlay = 0;
        }

        handDeltas[l] = deltaHand;
        playDeltas[l] = deltaLastPlay;
    }

	/* Aplicar deltas às posições originais */

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

            for (k = 0; k < 4; k++) { /* Percorrer todas as mãos / últimas jogadas e descobrir se a carta pertence a uma delas */

                if (cardExists(gameState.hands[k], i, j)) {

                    if (k == 0) {

                        /* Se a carta for do utilizador e estiver selecionada, imprime-se mais acima */
                        if (cardExists(gameState.selection, i, j)) {

                            printCard(path, handx[k], (handy[k] - 20), i, j, gameState, 2);

                        } else {

                            printCard(path, handx[k], handy[k], i, j, gameState, 2);
                        }

                        handx[k] += spaceBetweenCards; /* Incrementar o x para a próxima carta na mão de baixo */

                    } else if (k == 1) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 1);

                        handy[k] -= spaceBetweenCards;

                    } else if (k == 2) {

                        printCard(path, handx[k], handy[k], i, j, gameState, 0);

                        handx[k] -= spaceBetweenCards; /* Decrementar o x para a próxima carta na mão de cima */

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

	/* Imprimir botões */

	printf("<div id=\"button-container\">");

	/* Botão de jogar */

	char playStateString[10240];

	/* Se a seleção atual for jogável */
	if (isSelectionPlayable(gameState)) {

        state stateAfterPlay = gameState;

        stateAfterPlay.play = true;

        sprintf(playStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPlay));

        printf("<a href=\"%s\" class=\"btn green\">Jogar</a>", playStateString);

	} else {

        printf("<a href=\"#\" class=\"btn green disabled\">Jogar</a>");
	}

	/* Botão de passar */

	state stateAfterPass = gameState;

	stateAfterPass.pass = true;

	char passStateString[10240];

	sprintf(passStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterPass));

	printf("<a href=\"%s\" class=\"btn orange\">Passar</a>", passStateString);

	/* Botão de limpar */

	state stateAfterClear = gameState;

	stateAfterClear.selection = 0;

	char clearStateString[10240];

	sprintf(clearStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterClear));

	printf("<a href=\"%s\" class=\"btn purple\">Limpar</a>", clearStateString);

	/* Botão de recomeçar */

	printf("<a href=\"%s\" class=\"btn red\">Recomeçar</a>", SCRIPT);

	printf("</div>");
}

/** \brief Distribui cartas por 4 mãos aleatoriamente

    Esta função preenche um array com mãos selecionadas aleatoriamente

    @param hands     Array que vai ser preenchido com as mãos geradas aleatoriamente
*/
void distributeCards (long long int *hands) {

    /* Ter a certeza que as hands estão a zero */
    int m;
    for (m = 0; m < 4; m++) {
        hands[m] = 0;
    }

    /* Percorrer todos os naipes e cartas e atribuí-las a uma mão aleatória */

    int i, j, handSelected;

    /* char currentSuit, currentValue, currentCardIndex; */

    /* Mantém a conta de quantas cartas já foram para cada mão */
    int cardsInEachHand[4] = {0};

    for (i = 0; i < 4; i++) { /* Percorrer naipes */

            /* currentSuit = SUITS[i]; */

        for (j = 0; j < 13; j++) { /* Percorrer cartas */

            /* currentValue = VALUES[j]; */

            /* currentCardIndex = getCardIndex(i, j); */

            /* Repetir a escolha da mão até sair uma que não esteja completa */
            do {

                handSelected = rand() % 4;

            } while (cardsInEachHand[handSelected] == 13);

            /* Anotar que esta mão vai ter mais uma carta */
            cardsInEachHand[handSelected] += 1;

            /* Adicionar a carta à mao selecionada (hands é um array global) */
            hands[handSelected] = addCard(hands[handSelected], i, j);
        }
    }
}

/** \brief Avalia se uma jogada é maior que outra

    Não é garantido que esta função avalie se ambas as jogadas são válidas. Essa verificação deve ser feita noutro lugar.

    @param play1   A jogada que se quer descobrir se é maior
    @param play2   A jogada contra a qual se quer comparar
    @return        True se a primeira jogada for maior
*/
bool isPlayBigger (long long int play1, long long int play2) {

    /* A primeira coisa a fazer é verificar o tamanho das jogadas */

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

        /* Quanto maior o poder de uma carta menor o valor do número que a representa
           Nota: Não estamos a ter em conta se as jogadas são válidas ou não
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

/** \brief Decide se a seleção atual do jogador é jogável

    @param gameState    O estado de jogo atual
    @return             True se a seleção for jogável
*/
bool isSelectionPlayable (state gameState) {

    if (getHandLength(gameState.selection) == 0) {

        return false;

    } else {

        /* Obter a jogada mais recente */

        long long int mostRecentPlay = 0; /* Vai ficar a 0 se não for encontrada uma jogada na última ronda */

        int i;
        for (i = 3; i > 0; i--) {

            if (gameState.lastPlays[i] != 0 && ~(gameState.lastPlays[i]) != 0) { /* ~(long long int) 0 significa que aquele jogador ainda não fez nada */

                mostRecentPlay = gameState.lastPlays[i];

                break; /* Esta little bitch fez-me perder um pouco de tempo por me ter esquecido de o escrever (>8-/) */
            }
        }

        /* TODO
           Verificar que, se o utilizador tem o 3 de ouros na mão, tem de o jogar
        */

        int mostRecentPlayLength = getHandLength(mostRecentPlay);
        int selectionLength = getHandLength(gameState.selection);

        /* TODO - Verificar que a seleção é um par ou um trio válido e retornar false caso não seja

        if (selectionLength == 2) {

        } else if (selectionLength == 3) {

        } */

        if (mostRecentPlay == 0) { /* Se a jogada mais recente for um passe ou não tiver havido jogada mais recente */

           return true;

        } else if (mostRecentPlayLength == selectionLength) { /* Se ambas as mãos têm o mesmo tamanho */

            /* Retornar true se a seleção é maior que a jogada do bot que jogou anteriormente */
            return isPlayBigger(gameState.selection, mostRecentPlay);

        } else { /* Se a seleção não for válida */

            return false;
        }
    }
}

/** \brief Descobre quem tem o 3 de ouros (e por isso, joga primeiro)

    @param gameState    O estado de jogo atual
    @return             O índice da mão do jogador com o 3 de ouros
*/
int whoGoesFirst (state gameState) {

    int i;
    for (i = 0; i < 4; i++) { /* Percorrer as mãos dos jogadores */

        /* Descobrir quem tem o 3 de ouros */
        if (cardExists(gameState.hands[i], 0, 0)) {

            return i;
        }
    }
}

/** \brief Decide que jogada um bot deve fazer baseando-se no estado de jogo atual

    @param gameState    O estado de jogo atual
    @param index        O índice da mão do bot que está a jogar (no array hands do estado de jogo)
    @return             Uma mão que representa as cartas que devem ser jogadas. 0 significa um passe
*/
long long int chooseAIPlay (state gameState, int index) {

    /* Descobrir quais foram as jogadas anteriores */

    long long int lastPlays[3]; /* Array das últimas jogadas ordenadas da mais recente para a mais antiga */

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

    /* Descobrir qual foi a jogada válida mais recente */

    long long int mostRecentPlay = 0; /* Vai continuar a zero a menos que tenha havido uma jogada válida que não tenha sido um passe na última ronda */

    int i;
    for (i = 0; i < 3; i++) {

        /* Se:
            - Esta jogada foi uma jogada válida ((~(long long int) 0) significa que o jogador ainda não fez nada)
            - Esta jogada não foi um passe
        */
        if (~(lastPlays[i]) != 0 && lastPlays[i] != 0) {

            /* Anotar o valor da jogada mais recente que tenha sido válida e não tenha sido um passe */
            mostRecentPlay = lastPlays[i];

            break;
        }
    }

    /* Finalmente, decidir que jogada fazer */

    if (mostRecentPlay == 0) { /* Se não houve uma jogada na última ronda */

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

        /* Descobrir o número de cartas que temos de jogar */
        int numberOfCardsPlayed = getHandLength(mostRecentPlay);

        if (numberOfCardsPlayed == 1) {

            /* Jogar a carta válida mais baixa do baralho */
            int j, k;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        long long int possiblePlay = addCard((long long int) 0, k, j);

                        if (isPlayBigger(possiblePlay, mostRecentPlay)) { /* Se o bot que tem de jogar tem esta carta e é maior que a anterior */

                            /* Retornar a carta válida mais baixa */
                            return possiblePlay;
                        }
                    }
                }
            }

            /* Não encontramos uma jogada válida, logo passamos */
            return (long long int) 0;

        } else if (numberOfCardsPlayed == 2) {

            /* Jogar o par válido mais baixo do baralho */
            int j, k, l;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se não estamos no mesmo naipe da carta pela qual estamos a passar */

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

            /* Não encontramos uma jogada válida, logo passamos */
            return (long long int) 0;

        } else if (numberOfCardsPlayed == 3) {

            /* Jogar o trio válido mais baixo do baralho */
            int j, k, l, m;
            for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.hands[index], k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se não estamos no mesmo naipe da carta pela qual estamos a passar */

                                if (cardExists(gameState.hands[index], l, j)) { /* Se o bot tem esta carta */

                                    for (m = 0; m < 4; m++) { /* Tentar encontrar um trio (percorrer naipes) */

                                        if (m != k && m != l) { /* Se não estamos no mesmo naipe das cartas do par que encontramos */

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

            /* Não encontramos uma jogada válida, logo passamos */
            return (long long int) 0;

        } else {

            /* O bot não sabe jogar mais que 3 cartas iguais por agora */
            printf("<!-- An AI play was requested of more than 3 cards, and the bot didn't know what to do so it passed. -->");
            return (long long int) 0;
        }
    }
}

/** \brief Processa uma jogada do computador

    Sabendo qual é o bot que tem de jogar, decide que jogada fazer e processa-a

    @param gameState    O estado de jogo atual
    @param index        O índice da mão do bot que está a jogar (no array hands do estado de jogo)
    @return             O estado de jogo imediatamente após a jogada do computador
*/
state processBotAction (state gameState, int index) {

    /* Decidir que jogada fazer */
    long long int play = chooseAIPlay(gameState, index);

    gameState.lastPlays[index] = play;

    if (play != 0) { /* Se a jogada não for um passe (se for um passe não é preciso fazer nada) */

        /* Remover da mào do bot cada carta presente na sua jogada */
        int i, j;
        for (i = 0; i < 4; i++) { /* Percorrer naipes */
            for (j = 0; j < 13; j++) { /* Percorrer valores */

                if (cardExists(play, i, j)) { /* Se a carta fizer parte da jogada */

                    gameState.hands[index] = removeCard(gameState.hands[index], i, j); /* Removê-la da mào */
                }
            }
        }
    }

    return gameState;
}

/** \brief Processa uma jogada do utilizador

    Normalmente chamada depois de o utilizador clicar num botão (jogar ou passar)

    @param gameState    O estado de jogo atual
    @return             O estado de jogo imediatamente após a ação
*/
state processUserAction (state gameState) {

    if (gameState.pass) {

        /* Um 0 no array lastPlays implica um passe */
        gameState.lastPlays[0] = 0;

        /* Remover a ação de passar do estado de jogo */
        gameState.pass = false;

        return gameState; /* Parar a execução da função */
    }

    /* Se a ação não foi um passe, então o utilizador clicou no botão jogar */

    /* A seleção já deve ter sido validada antes de o utilizador carregar no botão de jogar, mas aqui fazemos um double check */
    if (!isSelectionPlayable(gameState)) {

        gameState.selection = 0;

        return gameState;
    }

    /* Colocar a jogada mais recente do utilizador no índice 0 do array lastPlays (correspondente à última jogada do utilizador) */
    gameState.lastPlays[0] = gameState.selection;

    /* Remover da mào do jogador cada carta presente na seleção */
    int i, j;
    for (i = 0; i < 4; i++) { /* Percorrer naipes */
        for (j = 0; j < 13; j++) { /* Percorrer valores */

            if (cardExists(gameState.selection, i, j)) { /* Se a carta estiver selecionada */

                gameState.hands[0] = removeCard(gameState.hands[0], i, j); /* Removê-la da mào */
            }
        }
    }

    /* Limpar a seleção do jogador */
    gameState.selection = 0;

    /* Limpar a ação jogar do estado de jogo */
    gameState.play = false;

    return gameState;
}

/** \brief Cria um estado de jogo inicial e retorna-o

    Esta função é normalmente usada no início de um jogo para criar um estado inicial

    @return     Um estado de jogo inicial
*/
state getInitialGameState () {

    state e;

    /* Distribuir cartas */
    distributeCards(e.hands);

    /* Todos os bits a 1 numa jogada significa que aquele jogador ainda não realizou nenhuma ação no jogo atual */
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

    Esta função recebe a query que é passada à cgi-bin e trata-a.

    @param query A query que é passada à cgi-bin
 */
void parse (char *query) {

	char stateString[1024];

	if(sscanf(query, "q=%s", stateString) == 1) {

        state gameState = stringToState(stateString);

        if (!gameState.play && !gameState.pass) {

            /* Se há uma string de parâmetros mas o utilizador não fez nada, imprimir o estado atual */
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

	    /* Obter um estado de jogo inicial com mãos baralhadas e valores por defeito */
	    state gameState = getInitialGameState();

	    /* Descobrir quem joga primeiro (quem tem o 3 de ouros) */
        int i = whoGoesFirst(gameState);

        /* Processar jogadas dos bots até ser a vez do utilizador */
        while (i > 0 && i < 4) {

            gameState = processBotAction(gameState, i);

            i++;
        }

		render(gameState);
	}
}

/** \brief Função principal

    Função principal do programa que imprime os cabeçalhos necessários e de seguida
    invoca a função que imprime o código html para desenhar as cartas
 */
int main () {

    /* Fornecer uma seed ao rand() */
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
