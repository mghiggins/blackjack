//
//  player.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_player_h
#define blackjack_player_h

#include <vector>
#include "gamefns.h"

using namespace std;

class player
{
    // abstract base player class. Players define strategy: choice on what action to take in the
    // game and also on how much to bet.
    
public:
    // actions are things players decide to do in a given game state
    enum action
    {
        Hit,
        Stand,
        Split,
        Double,
        Surrender
    };
    // action options are used in strategies to denote actions depending on whether
    // certain actions are available
    enum actionOptions
    {
        DoubleOrHit,
        DoubleOrStand,
        SurrenderOrHit,
        SurrenderOrStand
    };
    player() {};
    virtual ~player() {};
    
    void setMoney( double money ) { this->money = money; };
    double getMoney() const { return money; };
    void incrementMoney( double money ) { this->money += money; };
    
    virtual float getBet() = 0;
    
    virtual action chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender ) = 0;
    
protected:
    double money;
};

#endif
