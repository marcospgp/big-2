#include "Types.h"
#include "Logic.h"

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

    int j, k, l, cardsFound = 0, cardsFoundOfThisValue = 0;

    for (l = 0; l < 13; l++) { /* Percorrer valores */

        /* Contar o 2 como carta menor */
        if (l == 0) {

            j = 12; /* Começar pelo 2 */

        } else {

            j = l - 1;
        }

        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {

                cardsFound++;

                cardsFoundOfThisValue++;
            }
        }

        if (cardsFound > 0 && cardsFoundOfThisValue != 1) { /* Se não foi encontrada uma carta nesta iteração (ou foi encontrada mais que uma carta deste valor) e já tinham sido encontradas cartas */

            break; /* Prosseguir para o próximo caso */

        } else if (cardsFound == 5) {

            return true;

        } else {

            cardsFoundOfThisValue = 0; /* Dar reset ao valor */
        }
    }

    if (hand == (long long int) 299067162755601) { printf("<!-- 1 -->"); }

    /* Verificar o caso em que o às conta como carta menor */

    cardsFound = 0;
    cardsFoundOfThisValue = 0;

    for (l = 0; l < 13; l++) { /* Percorrer valores */

        /* Contar o 2 e o às como cartas menores */
        if (l == 0) {

            j = 11; /* Começar pelo às */

        } else if (l == 1) {

            j = 12; /* De seguida o 2 */

        } else {

            j = l - 2;
        }

        for (k = 0; k < 4; k++) { /* Percorrer naipes */

            if (cardExists(hand, k, j)) {



                cardsFound++;

                cardsFoundOfThisValue++;
            }
        }

        if (cardsFound > 0 && cardsFoundOfThisValue != 1) { /* Se não foi encontrada uma carta nesta iteração (ou foi encontrada mais que uma carta deste valor) e já tinham sido encontradas cartas */

            return false;

        } else if (cardsFound == 5) {

                return true;

        } else {

            cardsFoundOfThisValue = 0; /* Dar reset ao valor */
        }
    }

    printf("<!-- Warning: Expectedly unreachable code reached in isStraight -->");

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

        if (isStraight(play2) && !isFlush(play2)) { /* verifica se é straight mas não é straight flush */

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


        if (isFlush(play2) && !isStraight(play2)) { /* verifica se é flush mas não é straight flush */

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


        if (isFullHouse(play2)) {

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


        if (is4OfAKind(play2)) {

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

        if (isStraight(play2) && isFlush(play2)) {

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

/** \brief Descobre quem tem o 3 de ouros (e por isso, joga primeiro). Deve retornar 0 quando ninguém tem o 3 de ouros

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

    return 0;
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
