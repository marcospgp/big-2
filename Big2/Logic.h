#ifndef LOGIC_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

int getCardIndex (int suit, int value);

int getHandLength (long long int hand);

long long int addCard (long long int hand, int suit, int value);

long long int removeCard (long long int hand, int suit, int value);

int cardExists (long long int hand, int suit, int value);

void distributeCards (long long int *hands);

bool isStraight (long long int hand);

bool isFlush (long long int hand);

bool isFullHouse (long long int hand);

bool is4OfAKind (long long int hand);

int straightValue (long long int jogada);

int straightSuit (long long int jogada, int j);

int flushSuit (long long int jogada);

int flushValue (long long int jogada);

int fullHouseValue (long long int jogada);

int fourOfAKindValue (long long int jogada);

bool isPlayBigger (long long int play1, long long int play2);

bool isSelectionPlayable (state gameState);

int whoGoesFirst (state gameState);

long long int chooseAIPlay (state gameState, int index);

state processBotAction (state gameState, int index);

state processUserAction (state gameState);

state getInitialGameState ();

#endif
