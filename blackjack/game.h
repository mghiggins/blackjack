//
//  game.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_game_h
#define blackjack_game_h

#include "player.h"
#include "shoe.h"

class game
{
public:
    game( player& gamePlayer, int seed=1 );
    ~game() {};
    
    void runSingle();
    void runMultiple( int nRuns );
    
    int getSteps() const { return nSteps; };

    int nDecks;                 // number of decks - zero means an infinite number; cannot be negative
    bool standsOnSoft17;        // true, dealer stands on soft 17; false, dealer hits on soft 17
    bool doubleAfterSplit;      // true, doubles are allowed after splits
    bool surrendersAllowed;     // true, surrenders are ever allowed
    bool surrenderOnAce;        // true, surrenders are allowed against aces
    bool dealerPeeks;           // true, the dealer peeks before bets on whether he has a blackjack if a ten is up
    
    bool verbose;               // true, game state info is printed out during evaluation
    
private:
    player& gamePlayer;
    int nSteps;
    shoe gameShoe;
    int seed;
};

#endif
