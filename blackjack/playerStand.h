//
//  playerStand.h
//  blackjack
//
//  Created by Mark Higgins on 9/2/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_playerStand_h
#define blackjack_playerStand_h

#include "playerFixedBet.h"

class playerStand : public playerFixedBet
{
    // player that always stands
    
public:
    playerStand() {};
    virtual ~playerStand() {};
    
    virtual action chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender ) { return player::Stand; };
};


#endif
