#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
    URL da CGI
*/
#define SCRIPT		"http://127.0.0.1/cgi-bin/cartas"

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
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d"

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
	bool pass, play, sort;      /* Se o útlimo clique do utilizador representa uma ação */

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
        &e.play,
        &e.sort
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
        e.play,
        e.sort
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

/** \brief Avalia se uma seleção é um straight

    @param hand  A seleção que se pretende avaliar
    @return      True ou False
*/
bool isStraight (long long int hand) {

    /* Num straight o às pode tanto contar como a maior carta como a menor (A2345, 10JQKA) */
    /* Num straight o 2 conta como a carta abaixo do 3 */

    /* Verificar o caso em que o às conta como carta maior */

    /* Indice do valor do 2: 12 ("3456789TJQKA2") */

    int j, k, l, cardsFound = 0;
    bool cardsFoundOfThisValue = 0;

    for (l = 0; l < 13; l++) { /* Percorrer valores */

        /* Contar o 2 como carta menor */
        if (l == 0) {

            j = 12; /* Começar pelo 2 */

        } else {

            j -= 1;
        }

        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {

                cardsFound++;

                cardsFoundOfThisValue++;
            }
        }

        if (cardsFound > 0 && cardsFoundOfThisValue != 1) { /* Se não foi encontrada uma carta nesta iteração (ou foi encontrada mais que uma carta deste valor) e já tinham sido encontradas cartas */

            return false;

        } else {

            cardsFoundOfThisValue = 0; /* Dar reset ao valor */
        }
    }

    if (cardsFound == 5) {

        return true;
    }

    /* Verificar o caso em que o às conta como carta menor */

    cardsFound = 0;
    cardsFoundOfThisValue = 0;

    for (l = 0; l < 13; l++) { /* Percorrer valores */

        /* Contar o 2 e o às como cartas menores */
        if (l == 0) {

            j = 11; /* Começar pelo às */

        } else if (j == 1) {

            j = 12; /* De seguida o 2 */

        } else {

            j -= 2;
        }

        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {

                cardsFound++;

                cardsFoundOfThisValue++;
            }
        }

        if (cardsFound > 0 && cardsFoundOfThisValue != 1) { /* Se não foi encontrada uma carta nesta iteração (ou foi encontrada mais que uma carta deste valor) e já tinham sido encontradas cartas */

            return false;

        } else {

            cardsFoundOfThisValue = 0; /* Dar reset ao valor */
        }
    }

    return (cardsFound == 5);
}

/** \brief Avalia se uma seleção é um flush

    @param hand  A seleção que se pretende avaliar
    @return      True ou False
*/
bool isFlush (long long int hand) {

    int j, k, cardsFound = 0;

    for (k = 0; k < 4; k++) { /* Percorrer naipes */
        for (j = 0; j < 13; j++) { /* Percorrer valores */

            if (cardExists(hand, k, j)) {

                cardsFound++;
            }
        }

        if (cardsFound > 0 && cardsFound != 5) { /* Se foram encontradas cartas, mas não todas neste naipe */

            return false;
        }
    }

    return (cardsFound == 5);
}

/** \brief Avalia se uma seleção é um full house

    @param hand  A seleção que se pretende avaliar
    @return      True ou False
*/
bool isFullHouse (long long int hand) {

    int j, k, cardCount = 0, sameValueCardCount = 0;
    bool has3Cards = false, has2Cards = false;;

    for (j = 0; j < 13; j++) { /* Percorrer valores */
        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {

                cardCount++;
                sameValueCardCount++;
            }
        }

        if (sameValueCardCount == 3) {

            has3Cards = true;

        } else if (sameValueCardCount == 2) {

            has2Cards = true;
        }

        sameValueCardCount = 0;
    }

    return (cardCount == 5 && has3Cards && has2Cards);
}

/** \brief Avalia se uma seleção é um 4 of a kind

    @param hand  A seleção que se pretende avaliar
    @return      True ou False
*/
bool is4OfAKind (long long int hand) {

    int j, k, cardCount = 0, sameValueCardCount = 0;
    bool has4Cards = false;

    for (j = 0; j < 13; j++) { /* Percorrer valores */
        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {

                cardCount++;
                sameValueCardCount++;

                if (sameValueCardCount == 4) {
                    has4Cards = true;
                }

            }
        }

        sameValueCardCount = 0;
    }

    return (cardCount == 5 && has4Cards);
}


/** \brief Avalia o poder de um straight

    @param play1   A jogada da qual se quer obter a maior carta
    @return        Index (j) da carta de maior valor
*/
int straightValue (long long int jogada) {

    int j, k, cardCount = 0;

    for (j = 0; j < 11; j++) { /* so vai até ao 11 porque as únicas cartas que mantém o seu valor são do 3 ao K */
        
        for (k = 0; k < 4; k++) {

            if (cardExists(jogada, k, j)) {

                cardCount++;

            }
        
        }

        if ((j == 3) && (cardCount == 3)) { /* para o caso em que é "A 2 3 4 5" */

            return 2; /* index do 5 */

        }

        if ((j == 4) && (cardCount == 4)) { /* para o caso em que é "2 3 4 5 6" */

            return 3; /* index do 6 */

        }

        if ((j == 10) && (cardCount == 4)) { /* para o caso em que é "10 J Q K A" */

            return 11; /* index do Ás */

        }

        if (cardCount == 5) {

            return j;

        }
    }
}


/** \brief Avalia o suit de um straight que empatou

    @param play1   A jogada da qual se quer obter o maior suit
    @return        Valoração (k) - que vai de 0 a 3 - da carta de maior suit
*/
int straightSuit (long long int jogada, int j) {

    /* vai aumentando o k até encontrar a carta que existe na mao e dá o suit de 0 a 3, que quanto maior mais forte é */
    int k;
        
    for (k = 0; k < 4; k++) {

        if (cardExists(jogada, k, j)) {

            return k;

        }
    }
}


/** \brief Avalia o suit de um flush

    @param play1   A jogada da qual se quer obter o suit
    @return        0, 1, 2, 3 dependendo de suit
*/
int flushSuit (long long int jogada) {

    /* vai andar até encontrar uma quarta qualquer e dá o suit dela, porque são todas do mesmo suit */
    int j, k;
    
    for (j = 0; j < 13; j++) {   
        for (k = 0; k < 4; k++) {

            if (cardExists(jogada, k, j)) {

                return k;

            }
        }
    }
}


/** \brief Avalia o maior valor de um flush

    @param play1   A jogada da qual se quer obter o maior valor
    @return        0 até 12 dependendo do valor
*/
int flushValue (long long int jogada) {

    /* vai percorrer todas as cartas. quando encontrar a última, ou seja, cardCount = 5, dá o valor (j) dela */
    int j, k, cardCount = 0;

    for (j = 0; j < 13; j++) {
        
        for (k = 0; k < 4; k++) {

            if (cardExists(jogada, k, j)) {

                cardCount++;

            }
        }

        if (cardCount == 5) {

            return j;

        }
    }
}


/** \brief Avalia o maior valor do trio do full house

    @param play1   A jogada da qual se quer obter o maior valor
    @return        0 até 12 dependendo do valor
*/
int fullHouseValue (long long int jogada) {

    /* vai percorrer todas as cartas, até encontrar um valor com 3 cartas */
    int j, k, trio;

    for (j = 0; j < 13; j++) {
        
        for (k = 0, trio = 0; k < 4; k++) { /* reinicia trio sempre a 0, para procurar o valor (j) em que há 3 cartas */

            if (cardExists(jogada, k, j)) {

                trio++;

            }

            if (trio == 3) {

                return j;

            }
        }
    }
}


/** \brief Avalia o maior valor do quad do 4OfAKind

    @param play1   A jogada da qual se quer obter o maior valor
    @return        0 até 12 dependendo do valor
*/
int fourOfAKindValue (long long int jogada) {

    /* vai percorrer todas as cartas, até encontrar um valor com 4 cartas */
    int j, k, quad;

    for (j = 0; j < 13; j++) {
        
        for (k = 0, quad = 0; k < 4; k++) { /* reinicia quad sempre a 0, para procurar o valor (j) em que há 4 cartas */

            if (cardExists(jogada, k, j)) {

                quad++;

            }

            if (quad == 4) {

                return j;

            }
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


        if (isStraight(play2) & !isFlush(play2)) { /* verifica se é straight mas não é straight flush */

            if (isFlush(play1) || isFullHouse(play1) || is4OfAKind(play1)) { /* verifica se há maiores. se for straight flush, já entra no flush */
                return true;
            }

            else {
                
                if (straightValue(play1) > straightValue(play2)) { /* se o valor do straight da nova mao é maior que a anterior, então true */

                    return true;

                }

                else if (straightValue(play1) < straightValue(play2)) { /* se o valor do straight da nova mao é menor que a anterior, então false */

                    return false;

                }

                else { /* no caso de serem straights com cartas do mesmo valor */

                    if (straightSuit(play1, straightValue(play1)) > straightSuit(play2, straightValue(play2))) { /* se o naipe do straight novo é maior, dá true */

                        return true;

                    }

                    else { /* sendo menor, dá falso */

                        return false;

                    }

                }
            }
        }


        else if (isFlush(play2) && !isStraight(play2)) { /* verifica se é flush mas não é straight flush */

                if (isStraight(play1) && !isFlush(play1)) { /* verifica se é straight mas não é straight flush */
                    return false;
                }

                if (isFullHouse(play1) || is4OfAKind(play1) || (isStraight(play1) && isFlush(play1))) {
                    return true;
                }

                else {
                    
                    if (flushSuit(play1) > flushSuit(play2)) {

                        return true;

                    }

                    if (flushSuit(play1) < flushSuit(play2)) {

                        return false;

                    }

                    else {

                        if (flushValue(play1) > flushValue(play2)) {

                            return true;

                        }

                        else {

                            return false;

                        }


                    }
                }
            }


        else if (isFullHouse(play2)) {

                if ((isStraight(play1) && !isFlush(play1)) || (!isStraight(play1) && isFlush(play1))) {
                    return false;
                }

                if (is4OfAKind(play1) || (isStraight(play1) && isFlush(play1))) {
                    return true;
                }

                else {
                    
                    if (fullHouseValue(play1) > fullHouseValue(play2)) { /* se o valor das 3 cartas novas for maior */

                        return true;

                    }

                    else {

                        return false;

                    }
                }
            }


        else if (is4OfAKind(play2)) {

                if ((isStraight(play1) && !isFlush(play1)) || (!isStraight(play1) && isFlush(play1)) || isFullHouse(play1)) {
                    return false;
                }

                if (isStraight(play1) && isFlush(play1)) {
                    return true;
                }

                else {

                    if (fourOfAKindValue(play1) > fourOfAKindValue(play2)) { /* se o valor das 4 cartas novas for maior */

                        return true;

                    }

                    else {

                        return false;

                    }
                }
        }

        else if (isStraight(play2) && isFlush(play2)) {

                if (isStraight(play1) && isFlush(play1)) {
                    
                    if (straightValue(play1) > straightValue(play2)) {

                        return true;

                    }

                    if (straightValue(play1) < straightValue(play2)) {

                        return false;

                    }

                    else { /* caso as cartas todas do mesmo valor */

                        if (flushSuit(play1) > flushSuit(play2)) {

                            return true;

                        }

                        else {

                            return false;

                        }
                    }
                }

                else {
                    return false;
                }
            }

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
        for (i = 1; i < 4; i++) {

            if (gameState.lastPlays[i] != 0 && ~(gameState.lastPlays[i]) != 0) { /* ~(long long int) 0 significa que aquele jogador ainda não fez nada */

                mostRecentPlay = gameState.lastPlays[i];

                break; /* Esta little bitch fez-me perder um pouco de tempo por me ter esquecido de o escrever (>8-/) */
            }
        }

        int mostRecentPlayLength = getHandLength(mostRecentPlay);
        int selectionLength = getHandLength(gameState.selection);

        /* A jogada não é válida se não tiver tamanho igual à jogada mais recente */
        if (mostRecentPlayLength != selectionLength && mostRecentPlay != 0) {
            return false;
        }

        /* Verificar que, se o utilizador tem o 3 de ouros na mão, tem de o jogar */

        if (cardExists(gameState.hands[0], 0, 0) && !cardExists(gameState.selection, 0, 0)) {

            return false;
        }

        /* Verificar que a seleção é uma combinação válida */

        if (selectionLength == 4 || selectionLength > 5) {

            return false;

        } else if (selectionLength == 2 || selectionLength == 3) {

            /* Verificar que a seleção é um par ou um trio válido (só temos de verificar que as cartas são do mesmo valor) */

            int j, k, cardsFound = 0;

            for (j = 0; j < 13; j++) { /* Percorrer valores */

                for (k = 0; k < 4; k++) { /* Percorrer naipes */

                    if (cardExists(gameState.selection, k, j)) {

                        cardsFound++;
                    }
                }

                /* Se foram encontradas cartas, verificar que foram encontradas todas (dado que já percorremos o naipe todo) */
                if (cardsFound > 0 && cardsFound != selectionLength) {

                    return false;
                }
            }

        } else if (selectionLength == 5) {

            if (!( /* Se não for nenhuma mão conhecida */
                isStraight(gameState.selection)  ||
                isFlush(gameState.selection)     ||
                is4OfAKind(gameState.selection)  ||
                isFullHouse(gameState.selection)
            )) {

                return false;
            }
        }

        /* Verificar se a jogada (que já verificamos se é válida) pode ser jogada no contexto de jogo atual */

        if (mostRecentPlay == 0) { /* Se a jogada mais recente for um passe ou não tiver havido jogada mais recente */

           return true;

        } else {

            /* Retornar true se a seleção é maior que a jogada do bot que jogou anteriormente */
            return isPlayBigger(gameState.selection, mostRecentPlay);
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

    long long int lastPlays[3] = {0}; /* Array das últimas jogadas ordenadas da mais recente para a mais antiga */

    if (index == 0) {

        lastPlays[0] = gameState.lastPlays[1]; /* Para dar a dica (Vitor) */
        lastPlays[1] = gameState.lastPlays[2];
        lastPlays[2] = gameState.lastPlays[3];

    } else if (index == 1) {

        lastPlays[0] = gameState.lastPlays[2]; /* Para que seja possivel jogar com sentido horário (Vitor) */
        lastPlays[1] = gameState.lastPlays[3];
        lastPlays[2] = gameState.lastPlays[0];

    } else if (index == 2) {

        lastPlays[0] = gameState.lastPlays[3];
        lastPlays[1] = gameState.lastPlays[0]; /* Em todo este conjunto de instruções */
        lastPlays[2] = gameState.lastPlays[1];

    } else if (index == 3) {

        lastPlays[0] = gameState.lastPlays[0];
        lastPlays[1] = gameState.lastPlays[1];
        lastPlays[2] = gameState.lastPlays[2]; /* Para que seja possivel jogar com sentido horário (Vitor) */

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

    /* Descobrir que combinações de 5 cartas temos e podemos jogar */

    long long int hand = gameState.hands[index];
    int l = getHandLength(hand);

    int handArray[l][2]; /* Array da mão no formato [[naipe, valor]] */
    long long int fiveCardHands[160000] = {0}; /* Permutações de 5 cartas */

    /* Preencher handArray */

    int o, counter = 0;
    for (i = 0; i < 4; i++) { /* Percorrer naipes */
        for (o = 0; o < 13; o++) { /* Percorrer valores */

            if (cardExists(hand, i, o)) {

                handArray[counter][0] = i;
                handArray[counter][1] = o;

                counter++;
            }
        }
    }

    /* Percorrer todas as combinações possíveis de 5 cartas desta mão */

    int counters[5] = {0};
    counter = 0;
    l -= 1;

    while (!(counters[0] == l && counters[1] == l && counters[2] == l && counters[3] == l && counters[4] == l)) {

        /* Não podemos adicionar a mesma carta duas vezes */
        if (!(
            (counters[0] == counters[1] || counters[0] == counters[2] || counters[0] == counters[3] || counters[0] == counters[4]) ||
            (counters[1] == counters[2] || counters[1] == counters[3] || counters[1] == counters[4]) ||
            (counters[2] == counters[3] || counters[2] == counters[4]) ||
            (counters[3] == counters[4])
        )) {

            /* Adicionar esta carta às permutações */
            for (i = 0; i < 5; i++) {

                fiveCardHands[counter] = addCard(fiveCardHands[counter], handArray[counters[i]][0], handArray[counters[i]][1]);
            }

            counter++;
        }

        /* Incrementar os contadores */

        if (counters[4] < l) {

            counters[4]++;

        } else if (counters[3] < l) {

            counters[3]++;
            counters[4] = 0;

        } else if (counters [2] < l) {

            counters[2]++;
            counters[3] = 0;
            counters[4] = 0;

        } else if (counters [1] < l) {

            counters[1]++;
            counters[2] = 0;
            counters[3] = 0;
            counters[4] = 0;

        } else if (counters [0] < l) {

            counters[0]++;
            counters[1] = 0;
            counters[2] = 0;
            counters[3] = 0;
            counters[4] = 0;
        }
    }

    /* Finalmente, decidir que jogada fazer */

    /* Descobrir se se tem de jogar o 3 de ouros */
    bool mustPlay3OfDiamonds = cardExists(gameState.hands[index], 0, 0);

    /* Descobrir o tamanho do array fiveCardHands para quando o tivermos de percorrer*/
    int fiveCardHandsLength = (sizeof(fiveCardHands) / sizeof(fiveCardHands[0]));

    if (mostRecentPlay == 0) { /* Se não houve uma jogada na última ronda */

        /* Tentar jogar uma combinação */
        for (i = 0; i < fiveCardHandsLength; i++) {

            if (
                (isStraight(fiveCardHands[i])  || /* Verificar se esta combinação é uma mão válida */
                 isFlush(fiveCardHands[i])     ||
                 isFullHouse(fiveCardHands[i]) ||
                 is4OfAKind(fiveCardHands[i])) &&
                (!mustPlay3OfDiamonds || cardExists(fiveCardHands[i], 0, 0)) /* Verificar que se se tem o 3 de ouros, ele está na mão que se vai jogar */
            ) {
                return fiveCardHands[i];
            }
        }

        /* Jogar a carta mais baixa do baralho */
        int j, k;
        for (j = 0; j < 13; j++) { /* Percorrer valores primeiro (queremos a carta mais baixa) */
            for (k = 0; k < 4; k++) { /* Percorrer naipes */

                if (cardExists(hand, k, j)) { /* Se o bot que tem de jogar tem esta carta */

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

                    if (cardExists(hand, k, j)) { /* Se o bot tem esta carta */

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

                    if (cardExists(hand, k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se não estamos no mesmo naipe da carta pela qual estamos a passar */

                                if (cardExists(hand, l, j)) { /* Se o bot tem esta carta */

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

                    if (cardExists(hand, k, j)) { /* Se o bot tem esta carta */

                        for (l = 0; l < 4; l++) { /* Tentar encontrar um par (percorrer naipes) */

                            if (l != k) { /* Se não estamos no mesmo naipe da carta pela qual estamos a passar */

                                if (cardExists(hand, l, j)) { /* Se o bot tem esta carta */

                                    for (m = 0; m < 4; m++) { /* Tentar encontrar um trio (percorrer naipes) */

                                        if (m != k && m != l) { /* Se não estamos no mesmo naipe das cartas do par que encontramos */

                                            if (cardExists(hand, m, j)) { /* Se o bot tem esta carta */

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

        } else if (numberOfCardsPlayed == 5) {

            /* Tentar jogar uma combinação de 5 cartas */
            for (i = 0; i < fiveCardHandsLength; i++) {

                if (
                    (isStraight(fiveCardHands[i])  || /* Verificar se esta combinação é uma mão válida */
                     isFlush(fiveCardHands[i])     ||
                     isFullHouse(fiveCardHands[i]) ||
                     is4OfAKind(fiveCardHands[i])) &&
                     isPlayBigger(fiveCardHands[i], mostRecentPlay) && /* Verificar que esta possível jogada é maior que a anterior */
                    (!mustPlay3OfDiamonds || cardExists(fiveCardHands[i], 0, 0)) /* Verificar que se se tem o 3 de ouros, ele está na mão que se vai jogar (o que supostamente nunca vai acontecer aqui, mas sabe-se lá) */
                ) {
                    return fiveCardHands[i];
                }
            }

            return (long long int) 0; /* Se não se encontrou nenhuma combinação válida, passar */

        } else if (numberOfCardsPlayed == 4 || numberOfCardsPlayed > 5) {

            printf("<!-- Warning 01: A bot was asked to play against a hand of invalid size (in chooseAIPlay) -->");
            return (long long int) 0;

        } else {

            printf("<!-- Warning 02: A bot was asked to play against a hand of invalid size (in chooseAIPlay) -->");
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
    e.sort = false;

    return e;
}

/** \brief Imprime um estado de jogo

    Esta função imprime o estado atual do jogo no browser

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

/* TODO
    - METER APRESENTAÇÃO DE SCORE NO FINAL DO JOGO. vai ser preciso verificar se alguém tem 0 cartas e calcular o score no momento
*/

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

            if (getHandLength(gameState.hands[1]) != 0) { /* vê se o player 1 não ficou sem cartas. se ficou sem cartas, não deixa user jogar e faz render */

                /* Processar a jogada do utilizador */
                gameState = processUserAction(gameState);


                /* Processar a jogada dos bots */
            int i;
            for (i = 3; i > 0; i--) { /* para que seja possivel jogar com sentido horario (Vitor) */

                /* Este bloco define as mãos que vão ser analisadas, mediante o bot que estiver a jogar.
                   As mãos devem ser analisadas para o caso de alguma anterior ao bot que vai jogar tenha acabado o jogo.
                   Nesse caso, não se deixa alterar o gameState, fazendo o break e indo logo render.
                */
                int index;

                if (i == 3) {

                    index = 0;

                } else if (i == 2) {

                    index = 3;

                } else if (i == 1) {

                    index = 2;
                }

                if (getHandLength(gameState.hands[index]) != 0) {
                    gameState = processBotAction(gameState, i); /* ve se player 0, 3 ou 2 ficou sem cartas. se ficou da break e vai logo render */
                } else {
                    break;
                }
            }
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

            i--; /* para que seja possivel jogar com sentido horario (Vitor) */
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
	printf("Content-Type: text/html; charset=utf-8\n\n");
	printf("<head><title>Exemplo</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../big-2/style.css\">\n</head>\n");
	printf("<body>\n");

    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}



/*
separar_naipes vai contar o número de cartas de cada naipe. ex: [3,2,5,3]
                                                                 D,C,H,S
separar_valores vai contar o número de cartas de cada valor. ex: [1,0,3,0,1,2,1,0,2,0,1,2,0]
                                                                  3,4,5,6,7,8,9,T,J,Q,K,A,2

para straights a única coisa que mudar é em vez da lista acima [1,0,3,0,1,2,1,0,2,0,1,2,0], devemos ter uma lista que no fim conte de novo
tudo e mais o A de novo, pq estas jogadas podem terminar com A sendo a maior carta
*/
