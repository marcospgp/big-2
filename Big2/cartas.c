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
    Formato da string passada como parâmetro entre jogadas
*/
#define PARAMETER_STRING_FORMAT "%lld_%lld_%lld_%lld_%d_%d_%d_%d_%lld_%d_%d"

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

	long long int hands[4];  // Mãos dos 4 jogadores. A primeira deverá ser sempre a do utilizador, de modo a que sejá fácil averiguar que cartas tem num dado momento
	int cardCount[4];        // Número de cartas de cada jogador, na mesma ordem de hands[]
	long long int selection; // Cartas selecionadas atualmente pelo utilizador
	bool pass, play;         // Se o útlimo clique do utilizador representa uma ação

} state;

/** \brief Processa a string recebida como parâmetro e retorna o estado atual do jogo

    @param stateString  A string que contém a informação sobre o atual estado de jgoo
    @return     A informação contida na string recebida num formato utilizável em código
*/
state stringToState (char* str) {
	state e;
	sscanf(str, PARAMETER_STRING_FORMAT, &e.hands[0], &e.hands[1], &e.hands[2], &e.hands[3], &e.cardCount[0], &e.cardCount[1], &e.cardCount[2], &e.cardCount[3], &e.selection, &e.pass, &e.play);
	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return     Uma string que contém toda a informação do estado atual do jogo, pronta a ser usada como parâmetro
*/
char* stateToString (state e) {
	static char res[10240];
	sprintf(res, PARAMETER_STRING_FORMAT, e.hands[0], e.hands[1], e.hands[2], e.hands[3], e.cardCount[0], e.cardCount[1], e.cardCount[2], e.cardCount[3], e.selection, e.pass, e.play);
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

/** \brief Adiciona uma carta ao estado

    @param hand     A mão a ser modificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int addCard (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return hand | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

    @param hand     A mão a ser modificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		O novo estado
*/
long long int removeCard (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return hand & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

    @param hand     A mão a ser verificada
    @param naipe	O naipe da carta (inteiro entre 0 e 3)
    @param valor	O valor da carta (inteiro entre 0 e 12)
    @return		1 se a carta existe e 0 caso contrário
*/
int cardExists (long long int hand, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return (hand >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

    @param path	        O URL correspondente à pasta que contém todas as cartas
    @param x            A coordenada x da carta
    @param y            A coordenada y da carta
    @param naipe	    O naipe da carta (inteiro entre 0 e 3)
    @param valor	    O valor da carta (inteiro entre 0 e 12)
    @param gameState    O estado de jogo atual
*/
void printCard (char *path, int x, int y, int suit, int value, state gameState) {

	// Criar um estado que será usado se o utilizador clicar nesta carta

	state stateAfterClick = gameState;

    // Se a carta for do utilizador
    if (cardExists(gameState.hands[0], suit, value)) {

        // Se a carta já está selecionada
        if (cardExists(gameState.selection, suit, value)) {

            // Ao clicar nela será descelecionada
            stateAfterClick.selection = removeCard(stateAfterClick.selection, suit, value);

        } else {

            // Ao clicar nela será selecioanda
             stateAfterClick.selection = addCard(stateAfterClick.selection, suit, value);
        }
    } // Else, clicar na carta não faz nada

	// Criar url que será usado se esta carta for clicada, usando o estado que já foi criado acima
	char onClickUrl[10240];

	sprintf(onClickUrl, "%s?q=%s", SCRIPT, stateToString(stateAfterClick));

	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", onClickUrl, x, y, path, VALUES[value], SUITS[suit]);
}

/** \brief Imprime um estado de jogo

    Esta função imprime o estado atual do jogo

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Nestes for loops, x e y referem-se às coordenadas onde vai ser imprimida a próxima carta

	int i, j, k, x, y;

	for (y = 50, i = 0; i < 4; i++, y += 120) { // Percorrer mãos pelo eixo y

        // Percorrer todas as cartas e imprimir as que estiverem na mão

        x = 10; // Coordenada x inicial

		for (j = 0; j < 4; j++) { // Percorrer naipes

            for (k = 0; k < 13; k++) { // Percorrer valores

                if (cardExists(gameState.hands[i], j, k)) { // Se a carta estiver na mão

                    x += 30;

                    // Se a carta for do utilizador e estiver selecionada
                    // (bastava verificar se a carta está selecionada, mas assim se a mão a ser imprimida
                    // não for a do jogador passamos logo à frente)
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

/** \brief Cria um estado de jogo inicial e retorna-o

    Esta função é normalmente usada no início de um jogo para criar um estado inicial
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

	if(sscanf(query, "q=%s", &stateString) == 1) {

        state gameState = stringToState(stateString);

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
	printf("<head><title>Exemplo</title></head>\n");
	printf("<body style=\"background-color: #007700; color: #ffffff;\">\n");

	printf("<h1>Exemplo de utilização</h1>\n");

    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
