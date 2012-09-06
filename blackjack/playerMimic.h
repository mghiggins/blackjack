//
//  playerMimic.h
//  blackjack
//
//  Created by Mark Higgins on 9/2/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_playerMimic_h
#define blackjack_playerMimic_h

#include "playerFixedBet.h"

class playerMimic : public playerFixedBet
{
    // mimics the dealer: hits on 16 or less, stands on 17 (incl soft 17)
public:
    playerMimic() {};
    ~playerMimic() {};
    
    virtual action chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender );
};


#endif
