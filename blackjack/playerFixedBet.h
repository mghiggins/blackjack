//
//  playerFixedBet.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_playerFixedBet_h
#define blackjack_playerFixedBet_h

#include "player.h"

class playerFixedBet : public player
{
    // player that always bets the same amount each round.
    
public:
    playerFixedBet() {};
    virtual ~playerFixedBet() {};
    
    void setBetSize( float betSize ) { this->betSize=betSize; };
    float getBetSize() const { return betSize; };
    
    virtual float getBet() { return betSize; };

protected:
    float betSize;
};

#endif
