//
//  shoe.cpp
//  blackjack
//
//  Created by Mark Higgins on 9/1/12.
//  Copyright (c) 2012 Mark Higgins. All rights reserved.
//

#include "shoe.h"

shoe::shoe( int seed ) : seed(seed)
{
    rng = new CRandomMersenne(seed);
}

shoe::~shoe()
{
    delete rng;
}

int shoe::getCard()
{
    return rng->IRandom(1,13);
}

vector<int> shoe::getCards( int nCards )
{
    vector<int> ret;
    for( int i=0; i<nCards; i++ )
        ret.push_back( rng->IRandom(1,13) );
    return ret;
}