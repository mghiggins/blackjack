//
//  playerMimic.cpp
//  blackjack
//
//  Created by Mark Higgins on 9/2/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include "playerMimic.h"
#include "gamefns.h"

player::action playerMimic::chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender )
{
    countDetails playerCount = getHandCount(playerCards);
    if( playerCount.count >= 17 )
        return player::Stand;
    else
        return player::Hit;
}