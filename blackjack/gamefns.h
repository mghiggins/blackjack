//
//  gamefns.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_gamefns_h
#define blackjack_gamefns_h

#include <vector>
#include <string>
#include <hash_map.h>

using namespace std;

// define a hash fn for string so that hash_map can use strings as keys

namespace __gnu_cxx
{
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& x ) const
        {
            return hash< const char* >()( x.c_str() );
        }
    };
}

bool isTen(int card);
bool isAce(int card);
string cardName(int card);

class countDetails
{
    // holds details of the count of cards in a player's hand
public:
    int count;
    bool isSoft;
    bool isPair;
};

countDetails getHandCount( vector<int>& cards );

// hard, soft, and pairCountProbs return the probabilities of being
// dealt the first two cards with the relevant counts. hardCountProbs indexes
// 0->14 correspond to hard counts of 5->19. softCountProbs indexes
// 0->8 correspond to soft counts of 13->21. pairCountProbs indexes
// 0->9 correspond to pairs As, 2s, ... , 10s.

vector<float> hardCountProbs();
vector<float> softCountProbs();
vector<float> pairCountProbs();

#endif
