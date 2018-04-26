///////////////////////////////////////////////////////////////////////////////////////////
//    MultiNEAT - Python/C++ NeuroEvolution of Augmenting Topologies Library
//
//    Copyright (C) 2012 Peter Chervenski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with this program.  If not, see < http://www.gnu.org/licenses/ >.
//
//    Contact info:
//
//    Peter Chervenski < spookey@abv.bg >
//    Shane Ryan < shane.mcdonald.ryan@gmail.com >
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// File:        Random.cpp
// Description: Definition for a class dealing with random numbers.
///////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "Random.h"
#include "Utils.h"

namespace NEAT
{


// Seeds the random number generator with this value
void RNG::Seed(long a_Seed)
{
#ifdef USE_BOOST_RANDOM
    gen.seed(a_Seed);
#endif

    srand(a_Seed);
}

void RNG::TimeSeed()
{
    auto now = boost::posix_time::second_clock::local_time();
    Seed(now.time_of_day().total_milliseconds());
}

// Returns randomly either 1 or -1
int RNG::RandPosNeg()
{
#ifdef USE_BOOST_RANDOM
    boost::random::uniform_int_distribution<> dist(0, 1);
    int choice = dist(gen);
#else
    int choice = rand() % 2;
#endif
    if (choice == 0)
        return -1;
    else
        return 1;
}

// Returns a random integer between X and Y
int RNG::RandInt(int aX, int aY)
{
#ifdef USE_BOOST_RANDOM
    boost::random::uniform_int_distribution<> dist(aX, aY);
    return dist(gen);
#else
    if (aX == aY)
    {
        return aX;
    }
    if (aX == (aY-1))
    {
        // for two consecutives, pick either with equal probability
        if (RandFloat() < 0.5)
        {
            return aX;
        }
        else
        {
            return aY;
        }
    }
    int denom = (aY - aX + 1);
    if (denom == 0)
    {
        return 0;
    }
    return aX + (rand() % denom);
#endif
    
}

// Returns a random number from a uniform distribution in the range of [0 .. 1]
double RNG::RandFloat()
{
#ifdef USE_BOOST_RANDOM
    boost::random::uniform_01<> dist;
    return dist(gen);
#else 
    return (double)(rand() % RAND_MAX) / RAND_MAX;
#endif
}

// Returns a random number from a uniform distribution in the range of [-1 .. 1]
double RNG::RandFloatSigned()
{
    return (RandFloat() - RandFloat());
}

// Returns a random number from a gaussian (normal) distribution in the range of [-1 .. 1]
double RNG::RandGaussSigned()
{
#ifdef USE_BOOST_RANDOM
    boost::random::normal_distribution<> dist;
    double pick = dist(gen);
    Clamp(pick, -1, 1);
    return pick;
#else 
    static int t_iset=0;
    static double t_gset;
    double t_fac,t_rsq,t_v1,t_v2;
    
    if (t_iset==0)
    {
        do
        {
            t_v1=2.0*(RandFloat())-1.0;
            t_v2=2.0*(RandFloat())-1.0;
            t_rsq=t_v1*t_v1+t_v2*t_v2;
        }
        while (t_rsq>=1.0 || t_rsq==0.0);
    
        t_fac=sqrt(-2.0*log(t_rsq)/t_rsq);
        t_gset=t_v1*t_fac;
        t_iset=1;
    
        double t_tmp = t_v2*t_fac;
    
        Clamp(t_tmp, -1.0, 1.0);
        return t_tmp;
    }
    else
    {
        t_iset=0;
        double t_tmp = t_gset;
        Clamp(t_tmp, -1.0, 1.0);
        return t_tmp;
    }
#endif
}

int RNG::Roulette(const std::vector<double>& a_probs)
{
#ifdef USE_BOOST_RANDOM
    boost::random::discrete_distribution<> d_dist(a_probs);
    return d_dist(gen);
#else
    double t_marble = 0, t_spin = 0, t_total_score = 0;
    for(unsigned int i=0; i<a_probs.size(); i++)
    {
        t_total_score += a_probs[i];
    }
    t_marble = RandFloat() * t_total_score;
    
    int t_chosen = 0;
    t_spin = a_probs[t_chosen];
    while(t_spin < t_marble)
    {
        t_chosen++;
        t_spin += a_probs[t_chosen];
    }
    
    return t_chosen;
#endif
}


}
 // namespace NEAT
