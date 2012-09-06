//
//  gamefns.cpp
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include <sstream>
#include "gamefns.h"

bool isTen( int card )
{
    return card > 9 and card < 14;
}

bool isAce( int card )
{
    return card == 1;
}

string cardName( int card )
{
    if( card == 1 )
        return "Ace";
    else if( card > 1 and card < 11 )
    {
        stringstream ss;
        ss << card;
        return ss.str();
    }
    else if( card == 11 )
        return "Jack";
    else if( card == 12 )
        return "Queen";
    else if( card == 13 )
        return "King";
    else
        throw string("Unknown card!");
}

countDetails getHandCount( vector<int>& cards )
{
    // if there's only two cards and it's a double, return the count
    // and the pair flag.
    
    if( cards.size() == 2 and cards[0] == cards[1] )
    {
        countDetails dets;
        dets.isSoft = false;
        dets.isPair = true;
        
        if( isTen(cards[0]) )
            dets.count = 20;
        else if( isAce(cards[0]) )
            dets.count = 2;
        else
            dets.count = 2*cards[0];
        return dets;
    }
    
    // get the player's count and whether it's a soft or hard count,
    // and if it's a double if there's only two cards
    
    int count=0;
    bool isSoft=false;
    
    // we'll count any aces as 11, and then figure out later whether we can
    // use them as 1 - unless there's already an ace in there.
    
    for( int i=0; i<cards.size(); i++ )
    {
        if( isTen(cards[i]) )
            count += 10;
        else if( isAce(cards[i]) and not isSoft )
        {
            count += 11;
            isSoft = true;
        }
        else if( isAce(cards[i]) and isSoft )
            count += 1;
        else
            count += cards[i];
    }
    
    if( isSoft and count > 21 )
    {
        isSoft = false;
        count -= 10;
    }
    
    countDetails dets;
    dets.count  = count;
    dets.isSoft = isSoft;
    dets.isPair = false;
    
    return dets;
}

vector<float> hardCountProbs()
{
    vector<float> probs(15);
    probs[0]  = 2*1./13*1./13;  // 5: 3,2 and reverse
    probs[1]  = 2*1./13*1./13;  // 6: 4,2 and reverse
    probs[2]  = 4*1./13*1./13;  // 7: 5,2 and 4,3 and reverse
    probs[3]  = 4*1./13*1./13;  // 8: 6,2 and 5,3 and reverse
    probs[4]  = 6*1./13*1./13;  // 9: 7,2 and 6,3 and 5,4 and reverse
    probs[5]  = 6*1./13*1./13;  // 10: 8,2 and 7,3 and 6,4
    probs[6]  = 8*1./13*1./13;  // 11: 9,2 and 8,3 and 7,4 and 6,5 and reverse
    probs[7]  = 14*1./13*1./13; // 12: 10,2 (4 ways) and 9,3 and 8,4 and 7,5 and reverse
    probs[8]  = 14*1./13*1./13; // 13: 10,3 (4 ways) and 9,4 and 8,5 and 7,6 and reverse
    probs[9]  = 12*1./13*1./13; // 14: 10,4 (4 ways) and 9,5 and 8,6 and reverse
    probs[10] = 12*1./13*1./13; // 15: 10,5 (4 ways) and 9,6 and 8,7 and reverse
    probs[11] = 10*1./13*1./13; // 16: 10,6 (4 ways) and 9,7 and reverse
    probs[12] = 10*1./13*1./13; // 17: 10,7 (4 ways) and 9,8 and reverse
    probs[13] = 8*1./13*1./13;  // 18: 10,8 (4 ways) and reverse
    probs[14] = 8*1./13*1./13;  // 19: 10,9 (4 ways) and reverse
    return probs;
}

vector<float> softCountProbs()
{
    // all probs for aces are 2*1./13*1./13 except for 10s, where it's 4x as big.
    // Note that A+A is not included in the soft counts - it's included in the
    // pair counts.
    vector<float> probs(9,2*1./13*1./13);
    probs[8] *= 4; // Ace+10=21
    return probs;
}

vector<float> pairCountProbs()
{
    // all probs for pairs are 1./13*1./13 except for 10s, where it's 16x as big
    vector<float> probs(10,1./13*1./13);
    probs[9] *= 16; // 16 ways to get double ten-value cards out of the 169 possibilities
    return probs;
}