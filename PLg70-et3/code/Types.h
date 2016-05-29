#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

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

#endif
