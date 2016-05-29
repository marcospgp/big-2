#include "Types.h"
#include "View.h"

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

    printf("<svg width = \"1200\" height = \"800\">\n");

    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"1200\" style = \"fill:#007700\"/>\n");

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
    /*              mão 3              */
    /*       mão 4        mão 2        */
    /*              mão 1              */
    int hand1x = 400, hand1y = 650;
    int hand2x = 1065, hand2y = 520;
    int hand3x = (hand1x + (spaceBetweenCards * 12)), hand3y = 50;
    int hand4x = 85, hand4y = (hand2y - (spaceBetweenCards * 12)); /* As duas mãos laterais são imprimidas na vertical uma ao contrário da outra */

    int play1x = hand1x + 170, play1y = hand1y - 150;
    int play2x = hand2x - 250, play2y = hand2y - 190;
    int play3x = play1x, play3y = hand3y + 130;
    int play4x = play2x - 490, play4y = play2y;

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

    if (gameState.sort == 0) { /* para o default do sort=0, que vai meter ordem por valores @@@@@@@@@@@ VITOR */

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
    }

    else { /* para o caso de o jogador ter mudado a ordenação, tornando o valor do sort=1, que vai meter ordem por naipes @@@@@@@@@@@ VITOR */

        int i, j, k;

        for (i = 0; i < 4; i++) { /* Percorrer naipes- FOI TROCADA A ORDEM - VITOR */

            for (j = 0; j < 13; j++) { /* Percorrer valores - FOI TROCADA A ORDEM - VITOR */

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
    }

    /* Imprimir os textos "Passou" nos jogadores que passaram nesta jogada */
    int p;
    for (p = 0; p < 4; p++) {

        if (hasPassed[p] == true) {
            printPass(playx[p], playy[p] - 20);
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

    /* Botão de ordenar */

    char sortStateString[10240];

    state stateAfterSort = gameState;

    stateAfterSort.sort = !(stateAfterSort.sort);

    sprintf(sortStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterSort));

    printf("<a href=\"%s\" class=\"btn blue\">Ordem</a>", sortStateString);

    /* Botão de dica */

    state stateAfterTip = gameState;

    stateAfterTip.selection = chooseAIPlay(stateAfterTip, 0); /* Cria uma possível jogada, usando a função dos bots */

    char tipStateString[10240];

    sprintf(tipStateString, "%s?q=%s", SCRIPT, stateToString(stateAfterTip));

    printf("<a href=\"%s\" class=\"btn yellow\">Dica</a>", tipStateString);

    printf("</div>");

    /*
    int scorePlayer0 = (getHandLenght(gameState.hands[0] * (-1));
    int scorePlayer1 = (getHandLenght(gameState.hands[1] * (-1));
    int scorePlayer2 = (getHandLenght(gameState.hands[2] * (-1));
    int scorePlayer3 = (getHandLenght(gameState.hands[3] * (-1));

    printf("<a href=\"%d\" class=\"btn yellow\">%s - %d, %s - %d, %s - %d, %s - %d</a>", Score, "Player 0", scorePlayer0, "Player 1", scorePlayer1, "Player 2", scorePlayer2, "Player 3", scorePlayer3);

    printf para dar score.
    */

}
