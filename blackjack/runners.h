//
//  runners.h
//  blackjack
//
//  Created by Mark Higgins on 9/5/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#ifndef __blackjack__runners__
#define __blackjack__runners__

// runGame runs (in serial) a bunch of games and prints out the average return
// and the std dev of return

void runGame();

// runGameParallel runs (in parallel) a bunch of games and prints out the average
// return and the std dev of return

void runGameParallel( int nRuns=1000, int nDecks=0, bool standsOnSoft17=true, bool surrenderOnAce=true, bool surrendersAllowed=true,
                      bool doubleAfterSplit=true, bool dealerPeeks=true,
                      int nThreads=10, int initSeed=1 );

// printTable prints out the basic strategy tables for a given rule set

void printTable();

// compareTable prints out the differences in strategy actions between two
// different rule sets

void compareTable();

// calculateEdge calculates the edge (average return) from the player's perspective
// for a given rule set

void calculateEdge();

#endif /* defined(__blackjack__runners__) */
