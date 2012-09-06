//
//  shoe.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_shoe_h
#define blackjack_shoe_h

#include <vector>
#include "randomc.h"

using namespace std;

class shoe
{
    // the shoe that holds the decks of cards and deals cards.
    
public:
    shoe( int seed=1 );
    ~shoe();
    
    int getCard();
    vector<int> getCards( int nCards );
    
private:
    int seed;
    CRandomMersenne * rng;
};

#endif
