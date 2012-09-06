//
//  playerBasic.cpp
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include <list>
#include <string>
#include "playerBasic.h"
#include "gamefns.h"

float _hardVal( int playerCount, int dealerUpCard,
               vector< vector<bool> >& hardCalcd, vector< vector<bool> >& softCalcd,
               vector< vector<float> >& hardHit, vector< vector<float> >& hardStand,
               vector< vector<float> >& softHit, vector< vector<float> >& softStand,
               const vector< vector<float> >& dealerOdds );

float _softVal( int playerCount, int dealerUpCard,
               vector< vector<bool> >& hardCalcd, vector< vector<bool> >& softCalcd,
               vector< vector<float> >& hardHit, vector< vector<float> >& hardStand,
               vector< vector<float> >& softHit, vector< vector<float> >& softStand,
               const vector< vector<float> >& dealerOdds );

basicStrategyTable::basicStrategyTable( vector< vector<float> >& tableSoftHit, vector< vector<float> >& tableSoftStand,
                                        vector< vector<float> >& tableSoftDouble,
                                        vector< vector<float> >& tableHardHit, vector< vector<float> >& tableHardStand,
                                        vector< vector<float> >& tableHardDouble,
                                        vector< vector<float> >& tablePairHit, vector< vector<float> >& tablePairStand,
                                        vector< vector<float> >& tablePairDouble, vector< vector<float> >& tablePairSplit,
                                        vector< vector<float> >& dealerOdds )
: tableSoftHit(tableSoftHit), tableSoftStand(tableSoftStand), tableSoftDouble(tableSoftDouble),
  tableHardHit(tableHardHit), tableHardStand(tableHardStand), tableHardDouble(tableHardDouble),
  tablePairHit(tablePairHit), tablePairStand(tablePairStand), tablePairDouble(tablePairDouble), tablePairSplit(tablePairSplit),
  dealerOdds(dealerOdds)
{
    if( tableSoftHit.size() != 10 or tableSoftStand.size() != 10 or tableSoftDouble.size() != 10 )
        throw string("Soft table outer sizes must be 10");
    if( tableSoftHit[0].size() != 10 or tableSoftStand[0].size() != 10 or tableSoftDouble[0].size() != 10 )
        throw string("Soft table inner sizes must be 10");
    if( tableHardHit.size() != 18 or tableHardStand.size() != 18 or tableHardDouble.size() != 18 )
        throw string("Hard table outer sizes must be 18");
    if( tableHardHit[0].size() != 10 or tableHardStand[0].size() != 10 or tableHardDouble[0].size() != 10 )
        throw string("Hard table inner sizes must be 10");
    if( tablePairHit.size() != 10 or tablePairStand.size() != 10 or tablePairDouble.size() != 10 or tablePairSplit.size() != 10 )
        throw string("Pair table outer sizes must be 10");
    if( tablePairHit[0].size() != 10 or tablePairStand[0].size() != 10 or tablePairDouble[0].size() != 10 or tablePairSplit[0].size() != 10 )
        throw string("Pair table inner sizes must be 10");
    if( dealerOdds.size() != 10 ) throw string("Dealer odds outer size must be 10");
    if( dealerOdds[0].size() != 7 ) throw string("Dealer odds inner size must be 7");
}

class _valPair
{
public:
    _valPair( float val, player::action act ) : val(val), act(act) {};
    float val;
    player::action act;
};

player::action _getMax( float hitVal, float standVal, float doubleVal, float surrenderVal, float splitVal )
{
    if( hitVal>standVal and hitVal>doubleVal and hitVal>surrenderVal and hitVal>splitVal )
        return player::Hit;
    if( standVal>hitVal and standVal>doubleVal and standVal>surrenderVal and standVal>splitVal )
        return player::Stand;
    if( doubleVal>hitVal and doubleVal>standVal and doubleVal>surrenderVal and doubleVal>splitVal )
        return player::Double;
    if( surrenderVal>hitVal and surrenderVal>standVal and surrenderVal>doubleVal and surrenderVal>splitVal )
        return player::Surrender;
    return player::Split;
}

player::action basicStrategyTable::getActionSoft( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tableSoftHit.at(playerCount-12).at(dealerUpCount-1);
    float standVal = tableSoftStand.at(playerCount-12).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tableSoftDouble.at(playerCount-12).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    return _getMax(hitVal, standVal, doubleVal, surrVal, -1e50); // last is -1e50 since splits aren't allowed here
}

player::action basicStrategyTable::getActionHard( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tableHardHit.at(playerCount-4).at(dealerUpCount-1);
    float standVal = tableHardStand.at(playerCount-4).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tableHardDouble.at(playerCount-4).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    return _getMax(hitVal, standVal, doubleVal, surrVal, -1e50); // last is -1e50 since splits aren't allowed here
}

player::action basicStrategyTable::getActionPair( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tablePairHit.at(playerCount/2-1).at(dealerUpCount-1);
    float standVal = tablePairStand.at(playerCount/2-1).at(dealerUpCount-1);
    float splitVal = tablePairSplit.at(playerCount/2-1).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tablePairDouble.at(playerCount/2-1).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    return _getMax(hitVal, standVal, doubleVal, surrVal, splitVal);
}

float basicStrategyTable::getValueSoft( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tableSoftHit.at(playerCount-12).at(dealerUpCount-1);
    float standVal = tableSoftStand.at(playerCount-12).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tableSoftDouble.at(playerCount-12).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    if( hitVal>standVal and hitVal>doubleVal and hitVal>surrVal )
        return hitVal;
    if( standVal>hitVal and standVal>doubleVal and standVal>surrVal )
        return standVal;
    if( doubleVal>hitVal and doubleVal>standVal and doubleVal>surrVal )
        return doubleVal;
    return surrVal;
}

float basicStrategyTable::getValueHard( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tableHardHit.at(playerCount-4).at(dealerUpCount-1);
    float standVal = tableHardStand.at(playerCount-4).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tableHardDouble.at(playerCount-4).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    if( hitVal>standVal and hitVal>doubleVal and hitVal>surrVal )
        return hitVal;
    if( standVal>hitVal and standVal>doubleVal and standVal>surrVal )
        return standVal;
    if( doubleVal>hitVal and doubleVal>standVal and doubleVal>surrVal )
        return doubleVal;
    return surrVal;
};

float basicStrategyTable::getValuePair( int playerCount, int dealerUpCount, bool doubleAllowed, bool surrenderAllowed )
{
    float hitVal   = tablePairHit.at(playerCount/2-1).at(dealerUpCount-1);
    float standVal = tablePairStand.at(playerCount/2-1).at(dealerUpCount-1);
    float splitVal = tablePairSplit.at(playerCount/2-1).at(dealerUpCount-1);
    float doubleVal=-1e50, surrVal=-1e50;
    if( doubleAllowed )
        doubleVal = tablePairDouble.at(playerCount/2-1).at(dealerUpCount-1);
    if( surrenderAllowed )
        surrVal = -0.5;
    
    if( hitVal>standVal and hitVal>doubleVal and hitVal>surrVal and hitVal>splitVal )
        return hitVal;
    if( standVal>hitVal and standVal>doubleVal and standVal>surrVal and standVal>splitVal )
        return standVal;
    if( doubleVal>hitVal and doubleVal>standVal and doubleVal>surrVal and doubleVal>splitVal )
        return doubleVal;
    if( surrVal>hitVal and surrVal>standVal and surrVal>doubleVal and surrVal>splitVal )
        return surrVal;
    return splitVal;
};

bool basicStrategyKey::operator<( const basicStrategyKey& other ) const
{
    if( nDecks < other.nDecks ) return true;
    if( nDecks > other.nDecks ) return false;
    if( !standsOnSoft17 and other.standsOnSoft17 ) return true;
    if( standsOnSoft17 and !other.standsOnSoft17 ) return false;
    if( !doubleAfterSplit and other.doubleAfterSplit ) return true;
    if( doubleAfterSplit and !other.doubleAfterSplit ) return false;
    if( !surrendersAllowed and other.surrendersAllowed ) return true;
    if( surrendersAllowed and !other.surrendersAllowed ) return false;
    if( !dealerPeeks and other.dealerPeeks ) return true;
    if( dealerPeeks and !other.dealerPeeks ) return false;
    return false;
}

bool basicStrategyKey::operator==( const basicStrategyKey& other ) const
{
    return nDecks == other.nDecks and standsOnSoft17 == other.standsOnSoft17 and doubleAfterSplit == other.doubleAfterSplit
        and surrendersAllowed == other.surrendersAllowed and surrenderOnAce == other.surrenderOnAce
        and dealerPeeks == other.dealerPeeks;
}

vector<float> _dealerOddsForCount( int count, bool isSoft, bool standsOnSoft17, bool noTen, bool noAce )
{
    // recursive function to calculate odds of final count or bust. Doesn't allow
    // for blackjack outcome - that's assumed to have been handled at a higher level.
    // noTen and noAce might be true at the top level, for the first card, if the dealer's
    // peeked and seen no blackjack. Always false for deeper calls though.
    
    // probability of a specific card coming up. There are 13 different cards.
    
    if( noTen and noAce ) throw string("Cannot have noTen and noAce be true");
    
    float cardProb;
    if( noTen )
        cardProb = 1./9;
    else if( noAce )
        cardProb = 1./12;
    else
        cardProb = 1./13;
    
    vector<float> odds(6,0);
    
    // if the count is over 21, either it's a bust, or if it's a soft count then
    // it uses the hard count with count ten less.
    
    if( count > 21 )
    {
        if( isSoft )
            odds = _dealerOddsForCount(count-10,false,standsOnSoft17,false,false);
        else
            odds.at(5) = 1.; // bust
        return odds;
    }
    
    // if the count's 18 to 21, the dealer always stands (hard or soft)
    
    if( count > 17 )
    {
        odds.at(count-17) = 1.;
        return odds;
    }
    
    // if the count's 17 and it's hard, or if the game rules use a stand on a soft 17,
    // the dealer stands at 17.
    
    if( count == 17 and ( !isSoft or standsOnSoft17 ) )
    {
        odds.at(0) = 1.;
        return odds;
    }
    
    // otherwise average the odds over getting the nCards different cards dealt as the
    // dealer hits.
    
    int i, j;
    vector<float> subOdds;
    for( i=2; i<10; i++ )
    {
        subOdds = _dealerOddsForCount(count+i,isSoft,standsOnSoft17,false,false);
        for( j=0; j<6; j++ ) odds.at(j) += cardProb * subOdds.at(j);
    }
    
    if( !noTen )
    {
        subOdds = _dealerOddsForCount(count+10,isSoft,standsOnSoft17,false,false);
        for( j=0; j<6; j++ ) odds.at(j) += 4 * cardProb * subOdds.at(j);
    }
    
    // check what we do for an ace. If it's a hard count with total less than 12, it
    // switches to a soft count with ace counting as 11. If it's a hard count with
    // total 11 or bigger, it'll stay hard with ace as 1. If it's a soft count it
    // uses a soft count we always count the ace as 1 and it stays soft. That simplifies
    // to: count < 11, count the ace as 11 and switch to soft count; otherwise count
    // the ace as 1 and keep hard/soft unchanged.
    
    if( !noAce )
    {
        if( count < 11 ) // always hard
            subOdds = _dealerOddsForCount(count+11,true,standsOnSoft17,false,false);
        else
            subOdds = _dealerOddsForCount(count+1,isSoft,standsOnSoft17,false,false);
        
        for( j=0; j<6; j++ ) odds.at(j) += cardProb * subOdds.at(j);
    }
    
    return odds;
}

vector< vector<float> > basicStrategyFactory::getDealerOdds( basicStrategyKey& key )
{
    vector< vector<float> > odds(10);
    
    int i,j;
    for( i=0; i<10; i++ ) odds.at(i).resize(7,0);
    
    // first slot is for an ace up.
    
    vector<float> cardOdds;
    cardOdds = _dealerOddsForCount( 11, true, key.standsOnSoft17, true, false );
    
    if( key.dealerPeeks )
    {
        for( i=0; i<6; i++ ) odds.at(0).at(i) = cardOdds.at(i);
        odds.at(0).at(6) = 0; // zero probability of blackjack - this is conditioned on continuing
    }
    else
    {
        for( i=0; i<6; i++ ) odds.at(0).at(i) = cardOdds.at(i) * 9./13; // odds with no blackjack
        odds.at(0).at(6) = 4./13; // blackjack odds - getting any ten
    }
    
    // next slots are for 2->9 showing
    
    for( i=1; i<9; i++ )
    {
        cardOdds = _dealerOddsForCount( i+1, false, key.standsOnSoft17, false, false );
        for( j=0; j<6; j++ ) odds.at(i).at(j) = cardOdds.at(j);
        odds.at(i).at(6) = 0.; // zero probability of blackjack
    }
    
    // final slot is for a ten or face card showing
    
    cardOdds = _dealerOddsForCount( 10, false, key.standsOnSoft17, false, true );
    
    if( key.dealerPeeks )
    {
        for( i=0; i<6; i++ ) odds.at(9).at(i) = cardOdds.at(i);
        odds.at(9).at(6) = 0; // no blackjack if we're playing
    }
    else
    {
        for( i=0; i<6; i++ ) odds.at(9).at(i) = cardOdds.at(i) * 12./13; // odds with no blackjack
        odds.at(9).at(6) = 1./13; // blackjack odds - getting an ace
    }
    
    return odds;
}

float _hardVal( int playerCount, int dealerUpCard,
                vector< vector<bool> >& hardCalcd, vector< vector<bool> >& softCalcd,
                vector< vector<float> >& hardHit, vector< vector<float> >& hardStand,
                vector< vector<float> >& softHit, vector< vector<float> >& softStand,
                const vector< vector<float> >& dealerOdds )
{
    // if the player count > 21, it's a bust
    
    if( playerCount > 21 ) return -1;
    
    // if the table value is already calculated, return it
    
    double hitVal, standVal;
    
    if( hardCalcd.at(playerCount-4).at(dealerUpCard-1) )
    {
        hitVal   = hardHit.at(playerCount-4).at(dealerUpCard-1);
        standVal = hardStand.at(playerCount-4).at(dealerUpCard-1);
        return hitVal > standVal ? hitVal : standVal;
    }
    
    // otherwise calculate it. Start by calculating the stand value.
    
    int i;
    
    standVal = 0;
    for( i=0; i<5; i++ ) // dealer ends up not busting, not blackjacking, but with final count=17+i
    {
        if( playerCount>i+17 ) // player wins: player's count > dealer's final count
            standVal += 1 * dealerOdds.at(dealerUpCard-1).at(i);
        else if( playerCount==i+17 ) // push: player's count == dealer's final count
            standVal += 0;
        else // player loses: player's count < dealer's final count
            standVal -= 1 * dealerOdds.at(dealerUpCard-1).at(i);
    }
    standVal += dealerOdds.at(dealerUpCard-1).at(5); // odds of dealer busting
    standVal -= dealerOdds.at(dealerUpCard-1).at(6); // odds of dealer blackjacking
    
    hardStand.at(playerCount-4).at(dealerUpCard-1) = standVal;
    
    // now calculate the value on hitting - run through possible cards
    
    float cardProb=1./13, nextVal;
    hitVal = 0;
    
    // start with 2->9
    
    for( i=2; i<10; i++ )
    {
        nextVal = _hardVal( playerCount+i, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
        hitVal += cardProb * nextVal;
    }
    
    // then 10
    
    nextVal = _hardVal( playerCount+10, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    hitVal += 4 * cardProb * nextVal;
    
    // then ace. If the count is less than 11, adding an ace switches to a soft count; otherwise it's still a hard count.
    
    if( playerCount < 11 )
        nextVal = _softVal( playerCount+11, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    else
        nextVal = _hardVal( playerCount+1, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    
    hitVal += cardProb * nextVal;
    
    hardHit.at(playerCount-4).at(dealerUpCard-1) = hitVal;
    
    // note that this point is calculated
    
    hardCalcd.at(playerCount-4).at(dealerUpCard-1) = true;
    
    return hitVal > standVal ? hitVal : standVal;
}

float _softVal( int playerCount, int dealerUpCard,
               vector< vector<bool> >& hardCalcd, vector< vector<bool> >& softCalcd,
               vector< vector<float> >& hardHit, vector< vector<float> >& hardStand,
               vector< vector<float> >& softHit, vector< vector<float> >& softStand,
               const vector< vector<float> >& dealerOdds )
{
    // if the player count > 21, return the hard count val for count ten less
    
    if( playerCount > 21 )
        return _hardVal( playerCount-10, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    
    // if the table value is already calculated, return it
    
    double hitVal, standVal;
    
    if( softCalcd.at(playerCount-12).at(dealerUpCard-1) )
    {
        hitVal   = softHit.at(playerCount-12).at(dealerUpCard-1);
        standVal = softStand.at(playerCount-12).at(dealerUpCard-1);
        return hitVal > standVal ? hitVal : standVal;
    }
    
    // otherwise calculate it. Start by calculating the stand value.
    
    int i;
    
    standVal = 0;
    for( i=0; i<5; i++ ) // dealer ends up not busting, not blackjacking, but with final count=17+i
    {
        if( playerCount>i+17 ) // player wins: player's count > dealer's final count
            standVal += 1 * dealerOdds.at(dealerUpCard-1).at(i);
        else if( playerCount==i+17 ) // push: player's count == dealer's final count
            standVal += 0;
        else // player loses: player's count < dealer's final count
            standVal -= 1 * dealerOdds.at(dealerUpCard-1).at(i);
    }
    standVal += dealerOdds.at(dealerUpCard-1).at(5); // odds of dealer busting
    standVal -= dealerOdds.at(dealerUpCard-1).at(6); // odds of dealer blackjacking
    
    softStand.at(playerCount-12).at(dealerUpCard-1) = standVal;
    
    // now calculate the value on hitting - run through possible cards
    
    float cardProb=1./13, nextVal;
    hitVal = 0;
    
    // start with 2->9
    
    for( i=2; i<10; i++ )
    {
        nextVal = _softVal( playerCount+i, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
        hitVal += cardProb * nextVal;
    }
    
    // then 10
    
    nextVal = _softVal( playerCount+10, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    hitVal += 4 * cardProb * nextVal;
    
    // then ace.
    
    nextVal = _softVal( playerCount+1, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
    hitVal += cardProb * nextVal;
    
    softHit.at(playerCount-12).at(dealerUpCard-1) = hitVal;
    
    // note that this point is calculated
    
    softCalcd.at(playerCount-12).at(dealerUpCard-1) = true;
    
    return hitVal > standVal ? hitVal : standVal;
}

basicStrategyTable basicStrategyFactory::getTable( basicStrategyKey& key )
{
    hash_map<basicStrategyKey,basicStrategyTable>::iterator it = cachedTables.find( key );
    if( it == cachedTables.end() )
    {
        // table isn't already calculated - calculate it now
        
        // FIX FIX: for now we ignore the nDecks element and assume there is an infinite number of decks, so card play has no
        // impact on subsequent card probabilities
        
        // start by calculating the odds of the dealer getting various outcomes for each possible up card
        
        vector< vector<float> > dealerOdds = getDealerOdds(key);
        
        // set up the tables to hold the values we'll calculate
        
        int i,j,k,playerCount,dealerUpCard;
        float maxVal,dblVal,val;
        vector< vector<float> > hardHit(18), hardStand(18), hardDouble(18);
        vector< vector<float> > softHit(10), softStand(10), softDouble(10);
        vector< vector<bool> > hardCalcd(18), softCalcd(10);
        float cardProb=1./13;
        
        for( i=0; i<18; i++ )
        {
            hardHit.at(i).resize(10);
            hardStand.at(i).resize(10);
            hardDouble.at(i).resize(10);
            hardCalcd.at(i).resize(10,false);
            if( i<10 )
            {
                softHit.at(i).resize(10);
                softStand.at(i).resize(10);
                softDouble.at(i).resize(10);
                softCalcd.at(i).resize(10,false);
            }
        }
        
        for( i=17; i>=0; i-- ) // i is the player count - 4
            for( j=0; j<10; j++ ) // j is the dealer card: 0==ace, 1->9==dealer count-1
            {
                playerCount = i+4;
                dealerUpCard = j+1;
                
                maxVal = _hardVal( playerCount, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
                
                // calculate double value - equity doubles but only against one more card.
                
                dblVal = 0;
                
                // check values 2->9 for the next card, assuming we stand there
                
                for( k=2; k<10; k++ )
                {
                    if( playerCount+k<22 )
                        dblVal += 2 * cardProb * hardStand.at(i+k).at(j);
                    else
                        dblVal -= 2 * cardProb;
                }
                // then 10
                if( playerCount+10<22 )
                    dblVal += 2 * 4 * cardProb * hardStand.at(i+10).at(j);
                else
                    dblVal -= 2 * 4 * cardProb;
                // then ace - we're on a hard count - if the count is less than 11 then adding an ace turns it into a soft
                // count 11 larger. Otherwise it counts as a hard count one larger.
                if( playerCount<11 )
                {
                    val = _softVal( playerCount+11, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
                    dblVal += 2 * cardProb * softStand.at(playerCount+11-12).at(j);
                }
                else
                {
                    if( playerCount+1<22 )
                        dblVal += 2 * cardProb * hardStand.at(i+1).at(j);
                    else
                        dblVal -= 2 * cardProb;
                }
                
                hardDouble.at(i).at(j) = dblVal;
            }
        
        // then the soft counts
        
        for( i=9; i>=0; i-- ) // i is the player count - 12
            for( j=0; j<10; j++ ) // j is the dealer card: 0==ace, 1->9=dealer count-1
            {
                playerCount = i+12;
                dealerUpCard = j+1;
                
                maxVal = _softVal( playerCount, dealerUpCard, hardCalcd, softCalcd, hardHit, hardStand, softHit, softStand, dealerOdds );
                
                // calculate double value - equity doubles but only against one more card.
                
                dblVal = 0;
                
                // check values 2->9 for the next card, assuming we stand there. If the total soft count
                // ends up over 21 we switch to a hard count ten less (by making our ace worth 1 instead of 10).
                
                for( k=2; k<10; k++ )
                {
                    if( playerCount+k<22 )
                        dblVal += 2 * cardProb * softStand.at(i+k).at(j);
                    else
                        dblVal += 2 * cardProb * hardStand.at(playerCount+k-10-4).at(j);
                }
                // then 10
                if( playerCount+10<22 )
                    dblVal += 2 * 4 * cardProb * softStand.at(i+10).at(j);
                else
                    dblVal += 2 * 4 * cardProb * hardStand.at(playerCount+10-10-4).at(j);
                // then ace
                if( playerCount+1<22 )
                    dblVal += 2 * cardProb * softStand.at(i+1).at(j);
                else
                    dblVal += 2 * cardProb * hardStand.at(playerCount+1-10-4).at(j);
                
                softDouble.at(i).at(j) = dblVal;
            }

        // then the pair counts
        
        vector< vector<float> > pairHit(10), pairStand(10), pairDouble(10), pairSplit(10);
        for( i=0; i<10; i++ )
        {
            pairHit.at(i).resize(10);
            pairStand.at(i).resize(10);
            pairDouble.at(i).resize(10);
            pairSplit.at(i).resize(10);
        }
        
        float splitVal, sumLessMatch, nextVal, hitVal, standVal, surrVal;
        
        for( i=0; i<10; i++ ) // player card - 1: eg i=8 corresponds to a pair of 9s.
            for( j=0; j<10; j++ ) // dealer up card - 1
            {
                // get the stand value from the other stand values calculated
                
                if(i==0) // ace
                    pairStand.at(i).at(j) = softStand.at(0).at(j);
                else
                    pairStand.at(i).at(j) = hardStand.at(2*i-2).at(j);
                
                // get the hit value from the other hit values calculated
                
                if(i==0)
                    pairHit.at(i).at(j) = softHit.at(0).at(j);
                else
                    pairHit.at(i).at(j) = hardHit.at(2*i-2).at(j);
                
                // get the double value from the other double values calculated
                
                if(i==0)
                    pairDouble.at(i).at(j) = softDouble.at(0).at(j);
                else
                    pairDouble.at(i).at(j) = hardDouble.at(2*i-2).at(j);
                
                // calculate the split value. The calculation is a touch tricky because we average over possible
                // card combinations for each in the pair, but one of those combinations is the same pair we're
                // trying to estimate now. Two possibilities: the split value is larger than max(hit,stand,double)
                // or it's not. Calculate first assuming that it is, and then check to see if the calculated value
                // really is larger. If not, calculate assuming that it's smaller.
                
                sumLessMatch = 0; // sum of prob-weighted other values, less the one element where we get a pair again
                
                // first do 2->9
                
                for( k=2; k<10; k++ )
                {
                    // add to the running sum unless this card makes the same pair again
                    
                    if( k!=i+1 )
                    {
                        if( i==0 ) // soft count
                        {
                            hitVal   = softHit.at(11+k-12).at(j);
                            standVal = softStand.at(11+k-12).at(j);
                            dblVal   = softDouble.at(11+k-12).at(j);
                        }
                        else
                        {
                            hitVal   = hardHit.at(i+1+k-4).at(j);
                            standVal = hardStand.at(i+1+k-4).at(j);
                            dblVal   = hardDouble.at(i+1+k-4).at(j);
                        }
                        surrVal = -0.5;
                        
                        if( i==0 ) hitVal = dblVal = surrVal = -1e50; // must stand on an ace split
                        if( !key.doubleAfterSplit ) dblVal = -1e50; // never double
                        
                        if( hitVal>standVal and hitVal>dblVal and hitVal>surrVal )
                            nextVal = hitVal;
                        else if( standVal>hitVal and standVal>dblVal and standVal>surrVal )
                            nextVal = standVal;
                        else if( dblVal>hitVal and dblVal>standVal and dblVal>surrVal)
                            nextVal = dblVal;
                        else
                            nextVal = surrVal;
                            
                        sumLessMatch += 2*cardProb*nextVal;
                    }
                }
                
                // then 10
                
                if( i+1!=10 )
                {
                    if( i==0 ) // soft count
                    {
                        hitVal   = softHit.at(11+10-12).at(j);
                        standVal = softStand.at(11+10-12).at(j);
                        dblVal   = softDouble.at(11+10-12).at(j);
                    }
                    else
                    {
                        hitVal   = hardHit.at(i+1+10-4).at(j);
                        standVal = hardStand.at(i+1+10-4).at(j);
                        dblVal   = hardDouble.at(i+1+10-4).at(j);
                    }
                    
                    surrVal = -0.5;
                    
                    if( i==0 ) hitVal = dblVal = surrVal = -1e50; // must stand on an ace split
                    if( !key.doubleAfterSplit ) dblVal = -1e50; // never double
                    
                    if( hitVal>standVal and hitVal>dblVal and hitVal>surrVal )
                        nextVal = hitVal;
                    else if( standVal>hitVal and standVal>dblVal and standVal>surrVal )
                        nextVal = standVal;
                    else if( dblVal>hitVal and dblVal>standVal and dblVal>surrVal)
                        nextVal = dblVal;
                    else
                        nextVal = surrVal;

                    sumLessMatch += 2*4*cardProb*nextVal;
                }
                
                // then ace
                
                if( i!=0 )
                {
                    hitVal   = softHit.at(i+1+11-12).at(j);
                    standVal = softStand.at(i+1+11-12).at(j);
                    dblVal   = softDouble.at(i+1+11-12).at(j);
                    surrVal  = -0.5;
                    
                    if( i==0 ) hitVal = dblVal = surrVal = -1e50; // must stand on an ace split
                    if( !key.doubleAfterSplit ) dblVal = -1e50; // never double
                    
                    if( hitVal>standVal and hitVal>dblVal and hitVal>surrVal )
                        nextVal = hitVal;
                    else if( standVal>hitVal and standVal>dblVal and standVal>surrVal )
                        nextVal = standVal;
                    else if( dblVal>hitVal and dblVal>standVal and dblVal>surrVal)
                        nextVal = dblVal;
                    else
                        nextVal = surrVal;
                    
                    sumLessMatch += 2*cardProb*nextVal;
                }
                
                // calculate the value assuming the split val is greater than hit, stand, or double
                // (assumes we can't surrender after splitting).
                
                if( i+1==10 )
                    splitVal = sumLessMatch / ( 1 - 2*4*cardProb );
                else
                    splitVal = sumLessMatch / ( 1 - 2*cardProb );
                
                // if it's really larger than those three values, use it; otherwise calculate it
                // assuming it's not. Note that if we're splitting an ace we have to use the stand
                // value post-split.
                
                hitVal   = pairHit.at(i).at(j);
                standVal = pairStand.at(i).at(j);
                dblVal   = pairDouble.at(i).at(j);
                surrVal  = -0.5;
                
                if( i==0 ) hitVal = dblVal = surrVal = -1e50; // must stand on an ace split
                if( !key.doubleAfterSplit ) dblVal = -1e50; // never double
                
                if( hitVal>standVal and hitVal>dblVal and hitVal>surrVal )
                    nextVal = hitVal;
                else if( standVal>hitVal and standVal>dblVal and standVal>surrVal )
                    nextVal = standVal;
                else if( dblVal>hitVal and dblVal>standVal and dblVal>surrVal)
                    nextVal = dblVal;
                else
                    nextVal = surrVal;
                
                if( i!=0 and splitVal>nextVal )
                    pairSplit.at(i).at(j) = splitVal;
                else
                {
                    if( i+1==10 )
                        splitVal = sumLessMatch + 2*4*cardProb*nextVal;
                    else
                        splitVal = sumLessMatch + 2*cardProb*nextVal;
                    pairSplit.at(i).at(j) = splitVal;
                }
            }
        
        // construct the full table, cache it, and return it
        
        basicStrategyTable t( softHit, softStand, softDouble,
                              hardHit, hardStand, hardDouble,
                              pairHit, pairStand, pairDouble, pairSplit,
                              dealerOdds );
        cachedTables[key] = t;
        return t;
    }
    else
        // table is already calculated - return it
        return it->second;
}

player::action playerBasic::chooseAction( vector<int>& playerCards, int dealerUpCard, hash_map<string,float>& ctx, bool allowDouble, bool allowSurrender )
{
    // get some game-specific values out of the context
    
    int nDecks             = (int) ctx["nDecks"];
    bool standsOnSoft17    = ctx["standsOnSoft17"] == 1.;
    bool doubleAfterSplit  = ctx["doubleAfterSplit"] == 1.;
    bool surrendersAllowed = ctx["surrendersAllowed"] == 1.;
    bool surrenderOnAce    = ctx["surrenderOnAce"] == 1.;
    bool dealerPeeks       = ctx["dealerPeeks"] == 1.;
    
    // turn the options into a key for the caching of strategy tables
    
    basicStrategyKey key;
    key.nDecks            = nDecks;
    key.standsOnSoft17    = standsOnSoft17;
    key.doubleAfterSplit  = doubleAfterSplit;
    key.surrendersAllowed = surrendersAllowed;
    key.surrenderOnAce    = surrenderOnAce;
    key.dealerPeeks       = dealerPeeks;
    
    // get the strategy from the factory. Either grabs a pre-calculated table if it's been
    // calculated already, or calculates it and caches if it's the first evaluation.
    
    basicStrategyTable strat = stratFact.getTable( key );
    
    // get the count details from the player's hand
    
    countDetails dets = getHandCount(playerCards);
    
    // use the basic strategy to determine the best action. Doubles and surrenders are allowed
    // only if it's the first card the player calls; surrenders may have other constraints
    // depending on the choice of rules.
    
    int dealerCount;
    if( isTen(dealerUpCard) )
        dealerCount = 10;
    else if( isAce(dealerUpCard) )
        dealerCount = 1;
    else
        dealerCount = dealerUpCard;
    
    player::action ret;
    
    bool doubleAllowed = playerCards.size() == 2;
    bool surrenderAllowed = surrendersAllowed and playerCards.size() == 2;
    if( dealerCount==1 and not surrenderOnAce ) surrenderAllowed = false;
    
    if( dets.isPair )
        ret = strat.getActionPair( dets.count, dealerCount, doubleAllowed, surrenderAllowed );
    else if( dets.isSoft )
        ret = strat.getActionSoft( dets.count, dealerCount, doubleAllowed, surrenderAllowed );
    else
        ret = strat.getActionHard( dets.count, dealerCount, doubleAllowed, surrenderAllowed );
    
    return ret;
}