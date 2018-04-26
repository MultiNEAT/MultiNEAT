#ifndef _SPECIES_H
#define _SPECIES_H

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
// File:        Species.h
// Description: Definition for the Species class.
///////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "Innovation.h"
#include "Genome.h"
#include "Genes.h"

namespace NEAT
{

// forward
class Population;

//////////////////////////////////////////////
// The Species class
//////////////////////////////////////////////

class Species
{

    /////////////////////
    // Members
    /////////////////////

private:

    // ID of the species
    int m_ID;

    // Keep a local copy of the representative
    Genome m_Representative;

    // This tell us if this is the best species in the population
    bool m_BestSpecies;
    // This tell us if this is the worst species in the population
    bool m_WorstSpecies;


    // age of species (in generations)
    unsigned int m_AgeGenerations;
    // age of species (in evaluations)
    unsigned int m_AgeEvaluations;

    // how many of this species should be spawned for
    // the next population
    double m_OffspringRqd;

public:

    // best fitness found so far by this species
    double m_BestFitness;

    // Keep a local copy of the best genome
    // Useful in co-evolution
    Genome m_BestGenome;

    // generations since fitness has improved, we can use
    // this info to kill off a species if required
    unsigned int m_GensNoImprovement;
    // evaluations since fitness has improved
    unsigned int m_EvalsNoImprovement;

    // Color. Useful for displaying
    // Safe to access directly.
    int m_R, m_G, m_B;

    ////////////////////////////
    // Constructors
    ////////////////////////////

    // initializes a species with a leader genome and an ID number
    Species(const Genome& a_Seed, int a_id);

    // assignment 
    Species(const Species&) = default;
    Species& operator=(const Species& a_g) = default;
    
    // move
    Species(Species&&) = default;
    Species& operator=(Species&&) = default;

    // comparison operator (nessesary for boost::python)
    // todo: implement a better comparison technique
    bool operator==(Species const& other) const { return m_ID == other.m_ID; }

    ////////////////////////////
    // Destructor
    ////////////////////////////

    ////////////////////////////
    // Methods
    ////////////////////////////

    // Access
    double GetBestFitness() const { return m_BestFitness; }
    void SetBestSpecies(bool t) { m_BestSpecies = t; }
    void SetWorstSpecies(bool t) { m_WorstSpecies = t; }
    void IncreaseAgeGens() { m_AgeGenerations++; }
    void ResetAgeGens() { m_AgeGenerations = 0; m_GensNoImprovement = 0; }
    void IncreaseGensNoImprovement() { m_GensNoImprovement++; }
    void IncreaseAgeEvals() { m_AgeEvaluations++; }
    void ResetAgeEvals() { m_AgeEvaluations = 0; m_EvalsNoImprovement = 0; }
    void IncreaseEvalsNoImprovement() { m_EvalsNoImprovement++; }
    void SetOffspringRqd(double a_ofs) { m_OffspringRqd = a_ofs; }
    double GetOffspringRqd() const { return m_OffspringRqd; }
    unsigned int NumIndividuals() { return m_Individuals.size(); }
    void ClearIndividuals() { m_Individuals.clear(); }
    int ID() { return m_ID; }
    int GensNoImprovement() { return m_GensNoImprovement; }
    int EvalsNoImprovement() { return m_EvalsNoImprovement; }
    int AgeGens() { return m_AgeGenerations; }
    int AgeEvals() { return m_AgeEvaluations; }
    Genome GetIndividualByIdx(int a_idx) const { return (m_Individuals[a_idx]); }
    bool IsBestSpecies() const { return m_BestSpecies; }
    bool IsWorstSpecies() const { return m_WorstSpecies; }
    void SetRepresentative(Genome& a_G) { m_Representative = a_G; }

    // returns the leader (the member having the best fitness, representing the species)
    const Genome& GetLeader() const;

    const Genome& GetRepresentative() const;

    // adds a new member to the species and updates variables
    void AddIndividual(Genome& a_New);

    // returns an individual randomly selected from the best N%
    const Genome& GetIndividual(Parameters& a_Parameters, RNG& a_RNG) const;

    // returns a completely random individual
    const Genome& GetRandomIndividual(RNG& a_RNG) const;

    // calculates how many babies this species will spawn in total
    void CountOffspring();

    // this method performs fitness sharing
    // it also boosts the fitness if young and penalizes if old
    // applies extreme penalty for stagnating species over SpeciesDropoffAge generations.
    void AdjustFitness(Parameters& a_Parameters);

    // Sorts the individuals
    void SortIndividuals();




    ///////////////////////////////////////////////////
    // New stuff

    // each species CONTAINS the individuals
    std::vector<Genome> m_Individuals;

    // Reproduction.
    void Reproduce(Population& a_Pop, Parameters& a_Parameters, RNG& a_RNG);

    void MutateGenome( bool t_baby_is_clone, Population &a_Pop, Genome &t_baby, Parameters& a_Parameters, RNG& a_RNG);

    // Removes all individuals
    void Clear();


    ////////////////////////////////////////
    // Real-time methods

    double m_AverageFitness;

    // Computes an estimate of the average fitness
    void CalculateAverageFitness();

    // A second version that returns the baby only
    Genome ReproduceOne(Population& a_Pop, Parameters& a_Parameters, RNG& a_RNG);

    void RemoveIndividual(unsigned int a_idx);
};

    static inline void swap(Species &a, Species &b)
    {
        Species c(std::move(a));
        a = std::move(b);
        b = std::move(c);
    }
} // namespace NEAT

#endif


