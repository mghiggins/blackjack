//
//  runners.cpp
//  blackjack
//
//  Created by Mark Higgins on 9/5/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include <cmath>
#include <iostream>
#include <boost/thread.hpp>
#include "runners.h"
#include "playerBasic.h"
#include "game.h"

using namespace std;

void runGame()
{
    // define the player.
    
    double initMoney=100;
    playerBasic p;
    //playerStand p;
    //playerMimic p;
    p.setBetSize(1); // how much the player bets on each round
    
    // construct the game. This defines which variation of the rules we're using,
    // as well as things like minimum and maximum bets.
    
    int nSteps=100000000;
    int seed=4;
    game g(p,seed);
    //g.verbose = true;
    g.surrendersAllowed = false;
    g.dealerPeeks = false;
    
    double avgPNL=0,avgPNLSq=0,pnl;
    for( int i=0; i<nSteps; i++ )
    {
        if( i>0 and i%100000==0 ) cout << "Iteration " << i*1e-6 << "M; avg = " << avgPNL/i*100 << "%" << endl;
        p.setMoney(initMoney);
        g.runSingle();
        pnl = ( p.getMoney() - initMoney ) / p.getBetSize();
        avgPNL += pnl;
        avgPNLSq += pnl*pnl;
    }
    avgPNL /= nSteps;
    avgPNLSq /= nSteps;
    avgPNLSq -= avgPNL*avgPNL;
    double sdPNL = sqrt(avgPNLSq);
    cout << "Average return per game = " << avgPNL * 100 << "%\n";
    cout << "Std err on average      = " << sdPNL/sqrt(nSteps-1) * 100 << "%\n";
    cout << "Std dev per game        = " << sdPNL * 100 << "%\n";
}

vector<float> pnls;

class _runGameParallelWorker
{
public:
    _runGameParallelWorker( int i, int nRunsPerThread, const basicStrategyKey& key, int initSeed ) : i(i), nRunsPerThread(nRunsPerThread), key(key), initSeed(initSeed) {};
    
    void operator()()
    {
        playerBasic p;
        game g(p,initSeed+nRunsPerThread*i);
        g.nDecks            = key.nDecks;
        g.standsOnSoft17    = key.standsOnSoft17;
        g.surrenderOnAce    = key.surrenderOnAce;
        g.surrendersAllowed = key.surrendersAllowed;
        g.doubleAfterSplit  = key.doubleAfterSplit;
        g.dealerPeeks       = key.dealerPeeks;

        float pnl, initMoney=100;
        for( int run=0; run<nRunsPerThread; run++ )
        {
            p.setMoney(initMoney);
            p.setBetSize(1);
            g.runSingle();
            pnl = p.getMoney() - initMoney;
            pnls.at(i*nRunsPerThread+run) = pnl;
        }
    }
    
private:
    int i, nRunsPerThread;
    basicStrategyKey key;
    int initSeed;
};

void runGameParallel( int nRuns, int nDecks, bool standsOnSoft17, bool surrenderOnAce, bool surrendersAllowed,
                      bool doubleAfterSplit, bool dealerPeeks,
                      int nThreads, int initSeed )
{
    using namespace boost;
    
    basicStrategyKey key;
    key.nDecks            = nDecks;
    key.standsOnSoft17    = standsOnSoft17;
    key.surrenderOnAce    = surrenderOnAce;
    key.surrendersAllowed = surrendersAllowed;
    key.doubleAfterSplit  = doubleAfterSplit;
    key.dealerPeeks       = dealerPeeks;
    
    if( nRuns%nThreads != 0 ) throw string("Number of runs must be an integer multiple of number of threads");
    int nRunsPerThread = nRuns/nThreads;
    
    pnls.resize(nRuns);
    
    thread_group ts;
    for( int i=0; i<nThreads; i++ ) ts.create_thread( _runGameParallelWorker( i, nRunsPerThread, key, initSeed ) );
    ts.join_all();
    
    float avgPNL=0, avgPNLSq=0, pnl;
    for( int i=0; i<nRuns; i++ )
    {
        pnl = pnls.at(i);
        avgPNL += pnl;
        avgPNLSq += pnl*pnl;
    }
    avgPNL   /= nRuns;
    avgPNLSq /= nRuns;
    
    avgPNLSq -= avgPNL*avgPNL;
    double sdPNL = sqrt(avgPNLSq);
    cout << "Average return per game = " << avgPNL * 100 << "%\n";
    cout << "Std err on average      = " << sdPNL/sqrt(nRuns-1) * 100 << "%\n";
    cout << "Std dev per game        = " << sdPNL * 100 << "%\n";
}

void printTable()
{
    basicStrategyKey key;
    key.nDecks            = 0;
    key.standsOnSoft17    = true;
    key.surrenderOnAce    = true;
    key.surrendersAllowed = true;
    key.doubleAfterSplit  = true;
    key.dealerPeeks       = false;
    
    basicStrategyFactory f;
    /*
     vector< vector<float> > odds = f.getDealerOdds(key);
     
     int i, j;
     for( i=0; i<10; i++ )
     {
     cout << i+1 << ": ";
     for( j=0; j<7; j++ ) cout << odds.at(i).at(j) << " ";
     float ev = 2*odds.at(i).at(5) - 1;
     cout << ev << endl;
     cout << endl;
     }
     */
    
    basicStrategyTable t = f.getTable(key);
    
    cout << "HARD\n\n";
    
    int i, j;
    cout << "Pl  ";
    for( i=1; i<10; i++ )
    {
        if( i==9 )
            cout << "10 ";
        else
            cout << i+1 << "  ";
    }
    cout << "A\n";
    for( i=0; i<17; i++ )
    {
        if(i+5<10)
            cout << i+5 << ":  ";
        else
            cout << i+5 << ": ";
        for( j=1; j<10; j++ )
        {
            player::action act = t.getActionHard(i+5, j+1, true, true);
            if( act == player::Hit )
                cout << "H  ";
            else if( act == player::Stand )
                cout << "S  ";
            else if( act == player::Double )
                cout << "D  ";
            else if( act == player::Surrender )
                cout << "R  ";
            else
                cout << "?  ";
        }
        player::action act = t.getActionHard(i+5, 1, true, true);
        if( act == player::Hit )
            cout << "H  ";
        else if( act == player::Stand )
            cout << "S  ";
        else if( act == player::Double )
            cout << "D  ";
        else if( act == player::Surrender )
            cout << "R  ";
        else
            cout << "?  ";
        cout << endl;
    }
    cout << endl << endl;
    
    cout << "SOFT\n\n";
    
    cout << "Pl  ";
    for( i=1; i<10; i++ )
    {
        if( i==9 )
            cout << "10 ";
        else
            cout << i+1 << "  ";
    }
    cout << "A\n";
    for( i=0; i<9; i++ )
    {
        cout << i+13 << ": ";
        for( j=1; j<10; j++ )
        {
            player::action act = t.getActionSoft(i+13, j+1, true, true);
            if( act == player::Hit )
                cout << "H  ";
            else if( act == player::Stand )
                cout << "S  ";
            else if( act == player::Double )
                cout << "D  ";
            else if( act == player::Surrender )
                cout << "R  ";
            else
                cout << "?  ";
        }
        player::action act = t.getActionSoft(i+13, 1, true, true);
        if( act == player::Hit )
            cout << "H  ";
        else if( act == player::Stand )
            cout << "S  ";
        else if( act == player::Double )
            cout << "D  ";
        else if( act == player::Surrender )
            cout << "R  ";
        else
            cout << "?  ";
        cout << endl;
    }
    cout << endl << endl;
    
    cout << "PAIR\n\n";
    
    cout << "Pl  ";
    for( i=1; i<10; i++ )
    {
        if( i==9 )
            cout << "10 ";
        else
            cout << i+1 << "  ";
    }
    cout << "A\n";
    for( i=0; i<10; i++ )
    {
        if( i==0 )
            cout << "A:  ";
        else if( i+1==10 )
            cout << "10: ";
        else
            cout << i+1 << ":  ";
        for( j=1; j<10; j++ )
        {
            player::action act = t.getActionPair(2*(i+1), j+1, true, true);
            if( act == player::Hit )
                cout << "H  ";
            else if( act == player::Stand )
                cout << "S  ";
            else if( act == player::Double )
                cout << "D  ";
            else if( act == player::Surrender )
                cout << "R  ";
            else if( act == player::Split )
                cout << "P  ";
            else
                cout << "?  ";
        }
        player::action act = t.getActionPair(2*(i+1), 1, true, true);
        if( act == player::Hit )
            cout << "H  ";
        else if( act == player::Stand )
            cout << "S  ";
        else if( act == player::Double )
            cout << "D  ";
        else if( act == player::Surrender )
            cout << "R  ";
        else if( act == player::Split )
            cout << "P  ";
        else
            cout << "?  ";
        cout << endl;
    }
    cout << endl << endl;
    
    
    
    vector< vector<float> > s = t.tablePairSplit;
    for( i=0; i<10; i++ )
    {
        if(i+1<10)
            cout << i+1 << ":  ";
        else
            cout << i+1 << ": ";
        for( j=0; j<10; j++ )
            cout << s.at(i).at(j) << " ";
        cout << endl;
    }
    
}

void compareTable()
{
    basicStrategyKey key;
    key.nDecks            = 0;
    key.standsOnSoft17    = true;
    key.surrenderOnAce    = true;
    key.surrendersAllowed = true;
    key.doubleAfterSplit  = true;
    key.dealerPeeks       = true;
    
    basicStrategyFactory f;
    basicStrategyTable t1 = f.getTable(key);
    
    key.dealerPeeks = false;
    basicStrategyTable t2 = f.getTable(key);
    
    
    cout << "HARD\n\n";
    
    float pb = 2*1./13*4./13;
    
    int i, j;
    float v1, v2;
    for( i=0; i<17; i++ )
    {
        for( j=0; j<10; j++ )
        {
            player::action act1 = t1.getActionHard(i+5, j+1, true, true);
            player::action act2 = t2.getActionHard(i+5, j+1, true, true);
            if( act1 != act2 )
            {
                cout << i+5 << "," << j+1 << ": ";
                if( act1 == player::Hit )
                {
                    cout << "H  ";
                    v1 = t1.tableHardHit.at(i+5-4).at(j);
                }
                else if( act1 == player::Stand )
                {
                    cout << "S  ";
                    v1 = t1.tableHardStand.at(i+5-4).at(j);
                }
                else if( act1 == player::Double )
                {
                    cout << "D  ";
                    v1 = t1.tableHardDouble.at(i+5-4).at(j);
                }
                else if( act1 == player::Surrender )
                {
                    cout << "R  ";
                    v1 = -0.5;
                }
                else
                    cout << "?  ";
                
                if( act2 == player::Hit )
                {
                    cout << "H";
                    v2 = t2.tableHardHit.at(i+5-4).at(j);
                }
                else if( act2 == player::Stand )
                {
                    cout << "S";
                    v2 = t2.tableHardStand.at(i+5-4).at(j);
                }
                else if( act2 == player::Double )
                {
                    cout << "D";
                    v2 = t2.tableHardDouble.at(i+5-4).at(j);
                }
                else if( act2 == player::Surrender )
                {
                    cout << "R";
                    v2 = -0.5;
                }
                else
                    cout << "?";
                
                v1 = pb*(-1) + (1-pb)*v1;
                
                cout << "   " << v1 << "; " << v2 << "; " << v2-v1;
                cout << endl;
            }
        }
    }
    cout << endl << endl;
    
    cout << "SOFT\n\n";
    
    for( i=0; i<9; i++ )
    {
        for( j=0; j<10; j++ )
        {
            player::action act1 = t1.getActionSoft(i+13, j+1, true, true);
            player::action act2 = t2.getActionSoft(i+13, j+1, true, true);
            if( act1 != act2 )
            {
                cout << i+13 << "," << j+1 << ": ";
                if( act1 == player::Hit )
                {
                    cout << "H  ";
                    v1 = t1.tableSoftHit.at(i+13-12).at(j);
                }
                else if( act1 == player::Stand )
                {
                    cout << "S  ";
                    v1 = t1.tableSoftStand.at(i+13-12).at(j);
                }
                else if( act1 == player::Double )
                {
                    cout << "D  ";
                    v1 = t1.tableSoftDouble.at(i+13-12).at(j);
                }
                else if( act1 == player::Surrender )
                {
                    cout << "R  ";
                    v1 = -0.5;
                }
                else
                    cout << "?  ";
                
                if( act2 == player::Hit )
                {
                    cout << "H";
                    v2 = t2.tableSoftHit.at(i+13-12).at(j);
                }
                else if( act2 == player::Stand )
                {
                    cout << "S";
                    v2 = t2.tableSoftStand.at(i+13-12).at(j);
                }
                else if( act2 == player::Double )
                {
                    cout << "D";
                    v2 = t2.tableSoftDouble.at(i+13-12).at(j);
                }
                else if( act2 == player::Surrender )
                {
                    cout << "R";
                    v2 = -0.5;
                }
                else
                    cout << "?";
                
                v1 = pb*(-1) + (1-pb)*v1;
                
                cout << "   " << v1 << "; " << v2 << "; " << v2-v1;
                cout << endl;
            }
        }
    }
    cout << endl << endl;
    
    cout << "PAIR\n\n";
    
    for( i=0; i<10; i++ )
    {
        for( j=0; j<10; j++ )
        {
            player::action act1 = t1.getActionPair(2*(i+1), j+1, true, true);
            player::action act2 = t2.getActionPair(2*(i+1), j+1, true, true);
            if( act1 != act2 )
            {
                cout << i << "s," << j+1 << ": ";
                if( act1 == player::Hit )
                {
                    cout << "H  ";
                    v1 = t1.tablePairHit.at(i).at(j);
                }
                else if( act1 == player::Stand )
                {
                    cout << "S  ";
                    v1 = t1.tablePairStand.at(i).at(j);
                }
                else if( act1 == player::Double )
                {
                    cout << "D  ";
                    v1 = t1.tablePairDouble.at(i).at(j);
                }
                else if( act1 == player::Surrender )
                {
                    cout << "R  ";
                    v1 = -0.5;
                }
                else if( act1 == player::Split )
                {
                    cout << "P  ";
                    v1 = t1.tablePairSplit.at(i).at(j);
                }
                else
                    cout << "?  ";
                
                if( act2 == player::Hit )
                {
                    cout << "H";
                    v2 = t2.tablePairHit.at(i).at(j);
                }
                else if( act2 == player::Stand )
                {
                    cout << "S";
                    v2 = t2.tablePairStand.at(i).at(j);
                }
                else if( act2 == player::Double )
                {
                    cout << "D";
                    v2 = t2.tablePairDouble.at(i).at(j);
                }
                else if( act2 == player::Surrender )
                {
                    cout << "R";
                    v2 = -0.5;
                }
                else if( act2 == player::Split )
                {
                    cout << "P  ";
                    v1 = t2.tablePairSplit.at(i).at(j);
                }
                else
                    cout << "?";
                
                v1 = pb*(-1) + (1-pb)*v1;
                
                cout << "   " << v1 << "; " << v2 << "; " << v2-v1;
                cout << endl;
            }
        }
    }
    cout << endl << endl;
}

void calculateEdge()
{
    basicStrategyKey key;
    key.nDecks            = 0; // 0 means infinite decks
    key.standsOnSoft17    = true;
    key.surrenderOnAce    = true;
    key.surrendersAllowed = false;
    key.doubleAfterSplit  = true;
    key.dealerPeeks       = false;
    
    basicStrategyFactory f;
    basicStrategyTable t = f.getTable(key);
    
    double edge=0;
    
    vector<float> ph = hardCountProbs();
    vector<float> ps = softCountProbs();
    vector<float> pp = pairCountProbs();
    
    int i, j;
    double v, prob;
    
    // hard count - include everything from 5->19 (not 20 or 21 - can't get those with 2 cards & hard count)
    
    for( i=0; i<15; i++ )
        for( j=0; j<10; j++ )
        {
            v = t.getValueHard( i+5, j+1, true, key.surrendersAllowed );
            if( key.dealerPeeks )
            {
                if( j == 0 )
                    prob = 9./161;
                else if( j < 9 )
                    prob = 13./161;
                else
                    prob = 4*12./161;
            }
            else
            {
                if( j<9 )
                    prob = 1./13;
                else
                    prob = 4./13;
            }
            
            edge += ph.at(i)*v*prob;
        }
    
    // soft count - include everything but ace+10 - that's blackjack which we handle separately
    
    for( i=0; i<8; i++ )
        for( j=0; j<10; j++ )
        {
            v = t.getValueSoft( i+13, j+1, true, key.surrendersAllowed and key.surrenderOnAce );
            if( key.dealerPeeks )
            {
                if( j == 0 )
                    prob = 9./161;
                else if( j < 9 )
                    prob = 13./161;
                else
                    prob = 4*12./161;
            }
            else
            {
                if( j<9 )
                    prob = 1./13;
                else
                    prob = 4./13;
            }
            edge += ps.at(i)*v*prob;
        }
    
    // pairs - include everything
    
    for( i=0; i<10; i++ )
        for( j=0; j<10; j++ )
        {
            v = t.getValuePair(2*(i+1), j+1, true, key.surrendersAllowed and ( i!=0 or key.surrenderOnAce ) );
            if( key.dealerPeeks )
            {
                if( j == 0 )
                    prob = 9./161;
                else if( j < 9 )
                    prob = 13./161;
                else
                    prob = 4*12./161;
            }
            else
            {
                if( j<9 )
                    prob = 1./13;
                else
                    prob = 4./13;
            }
            edge += pp.at(i)*v*prob;
        }
    
    // handle blackjack by player & by dealer
    
    double pb=2*4./13*1./13;
    
    if( key.dealerPeeks )
    {
        // conditioned on neither getting blackjack, calculate the edge
        
        edge *= 169./161;
        
        // add in the contribution from player getting blackjack and dealer not, the reverse, and both getting blackjack
        
        edge = 1.5*pb*(1-pb) - 1*(1-pb)*pb + 0 * pb*pb + (1-pb)*(1-pb) * edge;
    }
    else
    {
        // the player gets paid 1.5 if he gets blackjack, but only if the dealer doesn't too.
        edge += 1.5*pb*(1-pb);
    }
    
    cout << "Average edge = " << edge*100 << "%" << endl;
}
