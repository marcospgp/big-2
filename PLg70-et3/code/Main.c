#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Types.h"
#include "Logic.h"
#include "View.h"

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

/** \brief Trata os argumentos da CGI

    Esta função recebe a query que é passada à cgi-bin e trata-a.

    @param query A query que é passada à cgi-bin
 */
void parse (char *query) {

	char stateString[1024];

	if(sscanf(query, "q=%s", stateString) == 1) {

        state gameState = stringToState(stateString);

        /* Verificar quem joga primeiro na mesma para permitir testar jogadas */

        int i = whoGoesFirst(gameState); /* Retorna 0 se ninguém tem o 3 de ouros */

        /* Processar jogadas dos bots até ser a vez do utilizador */
        while (i > 0 && i < 4) {

            gameState = processBotAction(gameState, i);

            i--; /* para que seja possivel jogar com sentido horario (Vitor) */
        }

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
	printf("<head><title>BIG 2</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../big-2/style.css\">\n</head>\n");
	printf("<body>\n");

    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
