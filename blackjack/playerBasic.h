//
//  playerBasic.h
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef blackjack_playerBasic_h
#define blackjack_playerBasic_h

#include <hash_map.h>
#include "playerFixedBet.h"

class basicStrategyTable
{
    // a cached basic strategy table
public:
    basicStrategyTable() {};
    basicStrategyTable( vector< vector<float> >& tableSoftHit, vector< vector<float> >& tableSoftStand,
                        vector< vector<float> >& tableSoftDouble,
                        vector< vector<float> >& tableHardHit, vector< vector<float> >& tableHardStand,
                        vector< vector<float> >& tableHardDouble,
                        vector< vector<float> >& tablePairHit, vector< vector<float> >& tablePairStand,
                        vector< vector<float> >& tablePairDouble, vector< vector<float> >& tablePairSplit,
                        vector< vector<float> >& dealerOdds );
    
    player::action getActionSoft( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    player::action getActionHard( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    player::action getActionPair( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    
    float getValueSoft( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    float getValueHard( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    float getValuePair( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed );
    
    // for the soft table of (player count,dealer count) -> value we have four tables
    // corresponding to the four possible actions: hit, stand, surrender, or double. Later
    // we will compare all four values to figure out the best action.
    // NOTE: actually we don't need a table for surrender, since the value under surrender
    // is always -0.5.
    
    vector< vector<float> > tableSoftHit;
    vector< vector<float> > tableSoftStand;
    vector< vector<float> > tableSoftDouble;
    
    // ditto for the hard table
    
    vector< vector<float> > tableHardHit;
    vector< vector<float> > tableHardStand;
    vector< vector<float> > tableHardDouble;
    
    // ditto for the pairs table, with the addition of a fifth possible action: splitting.
    
    vector< vector<float> > tablePairHit;
    vector< vector<float> > tablePairStand;
    vector< vector<float> > tablePairDouble;
    vector< vector<float> > tablePairSplit;
    
    // table that gets initialized with the odds of the dealer ending up with various final
    // counts or bust for various up cards.
    
    vector< vector<float> > dealerOdds;
};

class basicStrategyKey
{
public:
    int  nDecks;                // number of decks - zero means an infinite number; cannot be negative
    bool standsOnSoft17;        // true, dealer stands on soft 17; false, dealer hits on soft 17
    bool doubleAfterSplit;      // true, doubles are allowed after splits
    bool surrendersAllowed;     // true, surrenders are ever allowed
    bool surrenderOnAce;        // true, surrenders are allowed against aces
    bool dealerPeeks;           // true, the dealer peeks before bets on whether he has a blackjack if a ten is up
    
    bool operator<( const basicStrategyKey& other ) const;
    bool operator==( const basicStrategyKey& other ) const;
};

namespace __gnu_cxx
{
    template<> struct hash<basicStrategyKey>
    {
        size_t operator()( const basicStrategyKey& x ) const
        {
            int val = x.nDecks;
            if( x.standsOnSoft17 ) val += 100;
            if( x.doubleAfterSplit ) val += 1000;
            if( x.surrendersAllowed ) val += 10000;
            if( x.surrenderOnAce ) val += 100000;
            if( x.dealerPeeks ) val += 1000000;
            return hash<int>()( val );
        }
    };
}

class basicStrategyFactory
{
public:
    // getTable returns the basic strategy table for the game rules defined by
    // the basic strategy key. Either returns a cached value if it already exists,
    // or calculates the value, caches it, and then returns it.
    
    basicStrategyTable getTable( basicStrategyKey& key );
    
    // getDealerOdds is a helper function to calculate the odds of the different possible
    // dealer outcomes based on the dealer up card. Conditioned on playing the game: so
    // if the dealer peeks, the odds here assume that the dealer's hand was *not* blackjack.
    // Outer loop is the dealer up card: index 0 represents Ace, indexes 1->9 represent cards 2->10.
    // Inner loop represents outcome: indexes 0->4 represent 17->21, index 5 represents bust, and
    // index 6 represents blackjack.
    
    vector< vector<float> > getDealerOdds( basicStrategyKey& key );
    
private:
    hash_map<basicStrategyKey,basicStrategyTable> cachedTables;
};

class playerBasic : public playerFixedBet
{
    // player that follows the basic strategy
    
public:
    playerBasic() {};
    virtual ~playerBasic() {};
    
    virtual action chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender );
    
private:
    basicStrategyFactory stratFact;
};

#endif
