//
//  game.cpp
//  blackjack
//
//  Created by Mark Higgins on 8/28/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include <iostream>
#include <vector>
#include "game.h"
#include "gamefns.h"

using namespace std;



game::game( player& gamePlayer, int seed ) : gamePlayer(gamePlayer), nDecks(0), standsOnSoft17(true), doubleAfterSplit(true), surrendersAllowed(true), surrenderOnAce(true), dealerPeeks(true), nSteps(0), verbose(false), gameShoe(seed)
{
}

void _checkBlackjack( vector<int>& cards, bool verbose, player& gamePlayer, float bet )
{
    // player automatically loses unless he's also got a blackjack
    
    if( ( isTen(cards.at(0)) and isAce(cards.at(1)) ) or ( isTen(cards.at(1)) and isAce(cards.at(0)) ) )
    {
        if( verbose )
            cout << "Dealer blackjack, push against player blackjack\n";
        
        return;
    }
    else
    {
        if( verbose )
            cout << "Dealer blackjack, player loses\n";
        
        gamePlayer.incrementMoney(-bet);
        return;
    }
}

void game::runSingle()
{
    nSteps ++;
    
    // get the bet from the player
    
    float bet = gamePlayer.getBet();
    if( gamePlayer.getMoney() < bet ) return; // don't play if we've got less than one bet left
    
    // deal the cards. Dealer first; the first card is visible, the second is the hole card. These
    // are represented as a vector of ints.
    // Player cards are represented as a vector of vector of ints. Initially the outer vector has size 1
    // and represents the initial deal of two cards; it's a vector of vectors to allow for splits.
    
    vector<int> dealerCards( gameShoe.getCards(2) );
    if( verbose )
        cout << "Dealer: up card " << cardName(dealerCards.at(0)) << endl;
    vector< vector<int> > playerCards;
    playerCards.push_back( gameShoe.getCards(2) );
    if( verbose )
        cout << "Player: cards " << cardName(playerCards.at(0).at(0)) << "," << cardName(playerCards.at(0).at(1)) << endl;
    
    // if the dealer is allowed to peek, and his first card is a ten, check for an immediate dealer blackjack
    
    if( dealerPeeks and isTen(dealerCards.at(0)) and isAce(dealerCards.at(1)) )
    {
        if( verbose )
            cout << "Dealer: second card is an ace\n";
        _checkBlackjack(playerCards.at(0), verbose, gamePlayer, bet);
        return;
    }
    
    // FIX FIX: now we should check whether the dealer's up card is an ace and offer insurance to the player.
    // But practically it's never worthwhile to buy insurance so for now we skip.
    
    // if an ace is up and the dealer is peeking, check the other for a blackjack
    
    if( dealerPeeks and isAce(dealerCards.at(0)) and isTen(dealerCards.at(1)) )
    {
        if( verbose )
            cout << "Dealer: second card is a " << cardName(dealerCards.at(1)) << endl;
        _checkBlackjack(playerCards.at(0), verbose, gamePlayer, bet);
        return;
    }
    
    // player keeps playing until stand or bust. Note a "bust" flag for each pile the player
    // is playing (ie handling the different hands of a split).
    
    vector<bool> playerBust(playerCards.size(),false);
    vector<float> handBets(playerCards.size(),bet);
    vector<bool> playHand(playerCards.size(),true);
    
    int dealerUpCard;
    if( dealerCards.at(0) < 10 )
        dealerUpCard = dealerCards.at(0);
    else
        dealerUpCard = 10;
    hash_map<string,float> ctx;
    ctx["nDecks"]            = nDecks;
    ctx["standsOnSoft17"]    = standsOnSoft17 ? 1. : 0.;
    ctx["doubleAfterSplit"]  = doubleAfterSplit ? 1. : 0.;
    ctx["surrendersAllowed"] = surrendersAllowed ? 1. : 0.;
    ctx["surrenderOnAce"]    = surrenderOnAce ? 1. : 0.;
    ctx["dealerPeeks"]       = dealerPeeks ? 1. : 0.;
    
    int index=0;
    
    int i;
    while( index < playerCards.size() )
    {
        if( !playHand.at(index) )
        {
            if( verbose )
            {
                countDetails newCount = getHandCount( playerCards.at(index) );
                cout << "    Player hand " << index << " final count = " << newCount.count << endl;
            }
            index++;
            continue;
        }
        
        bool allowDouble=true, allowSurrender;
        int innerIndex=0;
        
        while( true )
        {
            innerIndex++;
            allowSurrender = surrendersAllowed;
            if( !surrenderOnAce )
                for( i=0; i<playerCards.at(index).size(); i++ )
                    if( isAce( playerCards.at(index).at(i) ) )
                    {
                        allowSurrender = false;
                        break;
                    }
            player::action act = gamePlayer.chooseAction( playerCards.at(index), dealerUpCard, ctx, allowDouble and innerIndex<2, allowSurrender and innerIndex<2 );
            if( act==player::Stand )
            {
                if( verbose )
                    cout << "Player hand " << index << ": stand\n";
                playerBust.at(index) = false;
                break;
            }
            else if( act==player::Hit )
            {
                int newCard=gameShoe.getCard();
                if( verbose )
                    cout << "Player hand " << index << ": hit - get card " << cardName(newCard) << endl;
                playerCards.at(index).push_back(newCard);
                countDetails newCount = getHandCount( playerCards.at(index) );
                if( newCount.count > 21 ) // bust
                {
                    if( verbose )
                        cout << "    Bust! Count is " << newCount.count << endl;
                    playerBust.at(index) = true;
                    break;
                }
            }
            else if( act==player::Double )
            {
                handBets.at(index) *= 2; // double the bet
                int newCard=gameShoe.getCard();
                if( verbose )
                    cout << "Player hand " << index << ": double, bet goes to " << handBets.at(index) << "; new card " << cardName(newCard) << endl;
                playerCards.at(index).push_back(newCard);
                countDetails newCount = getHandCount( playerCards.at(index) );
                if( newCount.count > 21 ) // bust
                {
                    if( verbose )
                        cout << "    Bust! Count is " << newCount.count << endl;
                    playerBust.at(index) = true;
                }
                break; // always stop after getting one more card
            }
            else if( act==player::Surrender )
            {
                handBets.at(index) *= 0.5; // halve the bet
                if( verbose )
                    cout << "Player hand " << index << ": surrender, losing " << handBets.at(index) << endl;
                playerBust.at(index) = true; // stop and lose
                break;
            }
            else if( act==player::Split )
            {
                // validate that a split makes sense, just in case
                
                if( playerCards.at(index).size() != 2 ) throw string("Cannot split with more than two cards");
                if( playerCards.at(index).at(0) != playerCards.at(index).at(1) ) throw string("Cannot split unless both cards are the same");
                
                // get two new cards and deal one to each hand
                
                vector<int> newCards = gameShoe.getCards(2);
                playerCards.at(index).at(1) = newCards.at(0);
                if( verbose )
                {
                    cout << "Player hand " << index << ": split\n";
                    cout << "    New hands: " << cardName(playerCards.at(index).at(0)) << "," << cardName(newCards.at(0))
                         << "; " << cardName(playerCards.at(index).at(0)) << "," << cardName(newCards.at(1)) << endl;
                }
                
                vector<int> nextCards(2);
                nextCards.at(0) = playerCards.at(index).at(0);
                nextCards.at(1) = newCards.at(1);
                playerCards.push_back(nextCards);
                playerBust.push_back(false);
                handBets.push_back(handBets.at(index));
                
                // now we continue - this will continue to deal out the cards to the first hand, then when that's
                // done will move on to deal out the second hand.
                // UNLESS, that is, the card we split was an ace, in which case we stop after one card.
                
                if( playerCards.at(index).at(0) == 1 )
                {
                    playHand.push_back(false);
                    break;
                }
                else
                    playHand.push_back(true);
            }
        }
        if( verbose )
        {
            countDetails newCount = getHandCount( playerCards.at(index) );
            cout << "    Player hand " << index << " final count = " << newCount.count << endl;
        }
        index ++;
    }
    
    // any hand that busted, take that money from the player
    
    for( i=0; i<playerCards.size(); i++ )
    {
        if( playerBust.at(i) )
            gamePlayer.incrementMoney(-handBets.at(i));
    }
    
    // now we've done dealing the player's cards - move on to the dealer
    
    if( verbose )
        cout << "Dealer: second card is " << cardName(dealerCards.at(1)) << endl;
    
    while( true )
    {
        // get the count with the current cards. If the count is more than 17, or
        // the count equals 17 and it's a hard count, or the dealer stands on a soft 17
        // and it's a soft count, then break. We check for a bust later.
        
        countDetails newCount = getHandCount( dealerCards );
        if( newCount.count > 17 ) break;
        if( newCount.count == 17 and ( standsOnSoft17 or !newCount.isSoft ) ) break;
        
        // deal a new card
        
        int newCard=gameShoe.getCard();
        dealerCards.push_back(newCard);
        if( verbose )
            cout << "Dealer: hits, new card " << cardName(newCard) << endl;
    }
    
    countDetails finalCount = getHandCount( dealerCards );
    if( verbose )
        cout << "    Dealer final count = " << finalCount.count << endl;
    
    if( finalCount.count == 21 )
    {
        // if the dealer got blackjack, he wins unless the player also got blackjack, in which
        // case it's a push
        
        if( dealerCards.size() == 2 )
        {
            if( verbose )
                cout << "    Dealer: got blackjack!\n";
            bool playerLoses=true;
            if( playerCards.size() == 1 and playerCards.at(0).size() == 2 and !playerBust.at(0) ) // 21 doesn't count as blackjack after splits
            {
                countDetails handCount = getHandCount( playerCards.at(0) );
                if( handCount.count == 21 )
                {
                    if( verbose )
                        cout << "      Player also got blackjack: push\n";
                    playerLoses = false;
                }
            }
            if( playerLoses )
                for( i=0; i<playerCards.size(); i++ )
                {
                    if( playerBust.at(i) ) continue; // already paid out for this
                    if( verbose )
                        cout << "      Player hand " << i << " loses " << handBets.at(i) << endl;
                    gamePlayer.incrementMoney(-handBets.at(i));
                }
        }
        else
        {
            // regular 21 - not blackjack - wins except against other 21s.
            
            for( i=0; i<playerCards.size(); i++ )
            {
                if( playerBust.at(i) ) continue; // already handled this case above
                
                countDetails handCount = getHandCount( playerCards.at(i) );
                if( handCount.count == 21 )
                {
                    if( playerCards.at(i).size() == 2 and playerCards.size() == 1 ) // blackjack
                    {
                        if( verbose )
                            cout << "    Player blackjack, wins " << handBets.at(i)*1.5 << endl;
                        gamePlayer.incrementMoney(handBets.at(i)*1.5);
                    }
                    else if( verbose )
                        cout << "    Player hand " << i << " has 21, push\n";
                }
                else
                {
                    if( verbose )
                        cout << "    Player hand " << i << " loses " << handBets.at(i) << endl;
                    gamePlayer.incrementMoney(-handBets.at(i));
                }
            }
        }
    }
    else if( finalCount.count > 21 )
    {
        // if the dealer has bust, the player wins a single unless he's already busted. Need
        // to check each of the hands (more than 1 if there were splits).
        
        if( verbose )
            cout << "    Bust! Dealer busts with count " << finalCount.count << endl;
        for( i=0; i<playerCards.size(); i++ )
            if( !playerBust.at(i) )
            {
                countDetails handCount = getHandCount( playerCards.at(i) );
                if( handCount.count==21 and playerCards.at(i).size() == 2 and playerCards.size() == 1 )
                {
                    if( verbose )
                        cout << "    Player hand " << i << " blackjack, wins " << handBets.at(i)*1.5 << endl;
                    gamePlayer.incrementMoney(handBets.at(i)*1.5);
                }
                else
                {
                    if( verbose )
                        cout << "    Player hand " << i << " wins " << handBets.at(i) << endl;
                    gamePlayer.incrementMoney(handBets.at(i));
                }
            }
    }
    else
    {
        // otherwise check whether it was a tie, win, or loss for the player
        
        for( i=0; i<playerCards.size(); i++ )
        {
            if( playerBust.at(i) ) continue; // lost money above
            
            countDetails handCount = getHandCount( playerCards.at(i) );
            if( handCount.count > finalCount.count )
            {
                if( handCount.count==21 and playerCards.at(i).size()==2 and playerCards.size() == 1 )
                {
                    if( verbose )
                        cout << "    Player hand " << i << " blackjack, wins " << handBets.at(i)*1.5 << endl;
                    gamePlayer.incrementMoney(handBets.at(i)*1.5);
                }
                else
                {
                    if( verbose )
                        cout << "    Player hand " << i << " wins " << handBets.at(i) << endl;
                    gamePlayer.incrementMoney(handBets.at(i));
                }
            }
            else if( handCount.count < finalCount.count )
            {
                if( verbose )
                    cout << "    Player hand " << i << " loses " << handBets.at(i) << endl;
                gamePlayer.incrementMoney(-handBets.at(i));
            }
            else
            {
                if( verbose )
                    cout << "    Player hand " << i << " pushes\n";
            }
        }
    }
}

void game::runMultiple( int nRuns )
{
    double initMoney = gamePlayer.getMoney();
    if( verbose ) cout << "Player starts with money " << initMoney << endl << endl;
    for( int i=0; i<nRuns; i++ )
    {
        if( i % 100000 == 0 )
            cout << "Iteration " << i << endl;
        
        if( verbose )
            cout << endl << "**** Game " << i+1 << "; player's profit " << gamePlayer.getMoney()-initMoney << endl;
        runSingle();
        if( gamePlayer.getMoney() <= 0 )
        {
            if( verbose )
                cout << "**** GAMBLER'S RUIN!! Out of money. :(\n";
            return;
        }
    }
}