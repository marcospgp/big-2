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

	long long int hands[4];  // Mãos dos 4 jogadores
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
	sscanf(str, PARAMETER_STRING_FORMAT, &e.hand[0], &e.hand[1], &e.hand[2], &e.hand[3], &e.cardCount[0], &e.cardCount[1], &e.cardCount[2], &e.cardCount[3], &e.selection, &e.pass, &e.play);
	return e;
}

/** \brief Codifica o estado atual do jogo numa string

    @param gameState    O estado de jogo atual
    @return     Uma string que contém toda a informação do estado atual do jogo, pronta a ser usada como parâmetro
*/
char* stateToString (state gameState) {
	static char res[10240];
	sprintf(res, PARAMETER_STRING_FORMAT, e.hand[0], e.hand[1], e.hand[2], e.hand[3], e.cardCount[0], e.cardCount[1], e.cardCount[2], e.cardCount[3], e.selection, e.pass, e.play);
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
    @return		1 se a carta existe e 0 caso contrário
*/
int cardExists (long long int ESTADO, int naipe, int valor) {
	int idx = getCardIndex(naipe, valor);
	return (ESTADO >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

    @param path	o URL correspondente à pasta que contém todas as cartas
    @param x A coordenada x da carta
    @param y A coordenada y da carta
    @param estados	As 4 mãos atuais
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

    Esta função imprime o estado atual do jogo

    @param gameState    estado atual do jogo
*/
void render (state gameState) {

	char *path = DECK;

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");

	// Nestes for loops, x e y referem-se às coordenadas onde vai ser imprimida a próxima carta

	int i, j, k, x, y;

	for (y = 10, i = 0; i < 4; i++, y += 120) { // Percorrer mãos pelo eixo y

        // Percorrer todas as cartas e imprimir as que estiverem na mão

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

/** \brief Distribui cartas pelas 4 mãos aleatoriamente

    Esta função preenche as mãos de um estado de jogo com cartas selecionadas aleatoriamente
*/
void distributeCards (state gameState) {




    // TODO





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

    // TODO

}

/** \brief Trata os argumentos da CGI

    Esta função recebe a query que é passada à cgi-bin e trata-a.

    @param query A query que é passada à cgi-bin
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
