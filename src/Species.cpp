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
// File:        Species.cpp
// Description: Implementation of the Species class.
///////////////////////////////////////////////////////////////////////////////



#include <algorithm>

#include "Genome.h"
#include "Species.h"
#include "Random.h"
#include "Population.h"
#include "Utils.h"
#include "Parameters.h"
#include "assert.h"

#define COMPAT_EQUALITY_DELTA 0.0000001

namespace NEAT
{
    RNG global_rng;
    
    bool genome_greater(const Genome& ls, const Genome& rs)
    {
        return (ls.GetFitness() > rs.GetFitness());
    }
    
    
    // initializes a species with a representative genome and an ID number
    Species::Species(const Genome &a_Genome, int a_ID)
    {
        m_ID = a_ID;
        //m_Individuals.reserve(50); wtf is this?
        // copy the initializing genome locally.
        // it is now the representative of the species.
        m_Representative = a_Genome;
        m_BestGenome = a_Genome;

        // add the first and only one individual
        m_Individuals.push_back(a_Genome);

        m_AgeGenerations = 0;
        m_GensNoImprovement = 0;
        m_OffspringRqd = 0;
        m_BestFitness = a_Genome.GetFitness();
        m_BestSpecies = true;
        m_WorstSpecies = false;
        m_AverageFitness = 0;

        // Choose a random color
        //RNG rng;
        //rng.TimeSeed();
        m_R = static_cast<int>(global_rng.RandFloat() * 255);
        m_G = static_cast<int>(global_rng.RandFloat() * 255) + 100;
        if (m_G > 255) m_G=255;
        m_B = static_cast<int>(global_rng.RandFloat() * 255);
    }

    // adds a new member to the species and updates variables
    void Species::AddIndividual(Genome &a_Genome)
    {
        m_Individuals.push_back(a_Genome);
    }


    // returns an individual randomly selected from the best N%
    const Genome& Species::GetIndividual(Parameters &a_Parameters, RNG &a_RNG) const
    {
        ASSERT(m_Individuals.size() > 0);

        // Make a pool of only evaluated individuals!
        std::vector<size_t> t_Evaluated;
        t_Evaluated.reserve(m_Individuals.size());
        for (size_t i = 0; i < m_Individuals.size(); i++)
        {
            if (m_Individuals[i].IsEvaluated())
            {
                t_Evaluated.push_back(i);
            }
        }

        ASSERT(t_Evaluated.size() > 0);

        if (t_Evaluated.size() == 1)
        {
            return m_Individuals[t_Evaluated[0]];
        }
        else if (t_Evaluated.size() == 2)
        {
            return m_Individuals[t_Evaluated[Rounded(a_RNG.RandFloat())]];
        }

        // Warning!!!! The individuals must be sorted by best fitness for this to work
        int t_chosen_one = 0;
        
        // Here might be introduced better selection scheme, but this works OK for now
        if (!a_Parameters.RouletteWheelSelection)
        {   //start with the last one just for comparison sake
            //int temp_genome;

            size_t t_num_parents = static_cast<size_t>(a_Parameters.SurvivalRate * t_Evaluated.size());
    
            ASSERT(t_num_parents > 0);
            ASSERT(t_num_parents < t_Evaluated.size());
            if (t_num_parents >= t_Evaluated.size())
            {
                t_num_parents = t_Evaluated.size() - 1;
            }
            t_chosen_one = a_RNG.RandInt(0, t_num_parents);
            
            /*for (unsigned int i = 0; i < a_Parameters.TournamentSize; i++)
            {
                temp_genome = a_RNG.RandInt(0, t_num_parents);

                if (m_Individuals[temp_genome].GetFitness() > m_Individuals[t_chosen_one].GetFitness())
                {
                    t_chosen_one = temp_genome;
                }
            }*/
        }
        else
        {
            // roulette wheel selection
            std::vector<double> t_probs;
            t_probs.reserve(t_Evaluated.size());
            for (unsigned int i = 0; i < t_Evaluated.size(); i++)
            {
                t_probs.push_back(m_Individuals[t_Evaluated[i]].GetFitness());
            }
            t_chosen_one = a_RNG.Roulette(t_probs);
        }

        return m_Individuals[t_Evaluated[t_chosen_one]];
    }


    // returns a completely random individual
    const Genome& Species::GetRandomIndividual(RNG &a_RNG) const
    {
        if (m_Individuals.size() == 0) // no members yet, return representative
        {
            return m_Representative;
        }
        else
        {
            int t_rand_choice = 0;
            t_rand_choice = a_RNG.RandInt(0, static_cast<int>(m_Individuals.size() - 1));
            return (m_Individuals[t_rand_choice]);
        }
    }

    // returns the leader (the member having the best fitness)
    const Genome& Species::GetLeader() const
    {
        // Don't store the leader any more
        // Perform a search over the members and return the most fit member

        // if empty, return representative
        if (m_Individuals.size() == 0)
        {
            return m_Representative;
        }

        double t_max_fitness = -99999999;
        int t_leader_idx = -1;
        for (unsigned int i = 0; i < m_Individuals.size(); i++)
        {
            double t_f = m_Individuals[i].GetFitness();
            if (t_max_fitness < t_f)
            {
                t_max_fitness = t_f;
                t_leader_idx = i;
            }
        }

        ASSERT(t_leader_idx != -1);
        return (m_Individuals[t_leader_idx]);
    }


    const Genome& Species::GetRepresentative() const
    {
        return m_Representative;
    }

    // calculates how many offspring this species should spawn
    void Species::CountOffspring()
    {
        m_OffspringRqd = 0;

        for (unsigned int i = 0; i < m_Individuals.size(); i++)
        {
            m_OffspringRqd += m_Individuals[i].GetOffspringAmount();
        }
    }


    // this method performs fitness sharing
    // it also boosts the fitness of the young and penalizes old species
    void Species::AdjustFitness(Parameters &a_Parameters)
    {
        ASSERT(m_Individuals.size() > 0);

        // iterate through the members
        for (unsigned int i = 0; i < m_Individuals.size(); i++)
        {
            double t_fitness = m_Individuals[i].GetFitness();

            // the fitness must be positive
            //DBG(t_fitness);
            ASSERT(t_fitness >= 0);

            // this prevents the fitness to be below zero
            if (t_fitness <= 0) t_fitness = 0.0001;

            // update the best fitness and stagnation counter
            if (t_fitness > m_BestFitness)
            {
                m_BestFitness = t_fitness;
                m_GensNoImprovement = 0;
            }

            // boost the fitness up to some young age
            if (m_AgeGenerations < a_Parameters.YoungAgeTreshold)
            {
                t_fitness *= a_Parameters.YoungAgeFitnessBoost;
            }

            // penalty for old species
            if (m_AgeGenerations > a_Parameters.OldAgeTreshold)
            {
                t_fitness *= a_Parameters.OldAgePenalty;
            }

            // extreme penalty if this species is stagnating for too long time
            // one exception if this is the best species found so far
            if (m_GensNoImprovement > a_Parameters.SpeciesMaxStagnation)
            {
                // the best species is always allowed to live
                if (!m_BestSpecies)
                {
                    // when the fitness is lowered that much, the species will
                    // likely have 0 offspring and therefore will not survive
                    t_fitness *= 0.0000001;
                }
            }

            // Compute the adjusted fitness for this member
            m_Individuals[i].SetAdjFitness(t_fitness / m_Individuals.size());
        }
    }


    void Species::SortIndividuals()
    {
        std::sort(m_Individuals.begin(), m_Individuals.end(), genome_greater);
    }


// Removes an individual from the species by its index within the species
    void Species::RemoveIndividual(unsigned int a_idx)
    {
        ASSERT(a_idx < m_Individuals.size());
        m_Individuals.erase(m_Individuals.begin() + a_idx);
    }

    // Reproduce mates & mutates the individuals of the species
    // It may access the global species list in the population
    // because some babies may turn out to belong in another species
    // that have to be created.
    // Also calls Birth() for every new baby
    void Species::Reproduce(Population &a_Pop, Parameters &a_Parameters, RNG &a_RNG)
    {
        Genome t_baby; // temp genome for reproduction

        int t_offspring_count = Rounded(GetOffspringRqd());
        int elite_offspring = Rounded(a_Parameters.EliteFraction * m_Individuals.size());
        if (elite_offspring < 1) // can't be 0
        {
            elite_offspring = 1;
        }
        // ensure we have a champ
        int elite_count = 0;
        // no offspring?! yikes.. dead species!
        if (t_offspring_count == 0)
        {
            // maybe do something else?
            return;
        }

        //////////////////////////
        // Reproduction

        // Spawn t_offspring_count babies
        //bool t_champ_chosen = false;
        bool t_baby_exists_in_pop = false;
        while (t_offspring_count--)
        {
            // Select the elite first..

            if (elite_count < elite_offspring)
            {
                t_baby = m_Individuals[elite_count];
                elite_count++;
            }
            else
            {
                do // - while the baby already exists somewhere in the new population or turned invalid in some way
                {
                    // this tells us if the baby is a result of mating
                    bool t_mated = false;

                    // There must be individuals there..
                    ASSERT(NumIndividuals() > 0);

                    // for a species of size 1 we can only mutate
                    // NOTE: but does it make sense since we know this is the champ?
                    if (NumIndividuals() == 1)
                    {
                        t_baby = GetIndividual(a_Parameters, a_RNG);
                        t_mated = false;
                    }
                        // else we can mate
                    else
                    {
                        Genome t_mom = GetIndividual(a_Parameters, a_RNG);

                        // choose whether to mate at all
                        // Do not allow crossover when in simplifying phase
                        if ((a_RNG.RandFloat() < a_Parameters.CrossoverRate) && (a_Pop.GetSearchMode() != SIMPLIFYING))
                        {
                            // get the father
                            Genome t_dad;
                            bool t_interspecies = false;

                            // There is a probability that the father may come from another species
                            if ((a_RNG.RandFloat() < a_Parameters.InterspeciesCrossoverRate) &&
                                (a_Pop.m_Species.size() > 1))
                            {
                                // Find different species (random one) // !!!!!!!!!!!!!!!!!
                                int t_diffspec = a_RNG.RandInt(0, static_cast<int>(a_Pop.m_Species.size() - 1));
                                t_dad = a_Pop.m_Species[t_diffspec].GetIndividual(a_Parameters, a_RNG);
                                t_interspecies = true;
                            }
                            else
                            {
                                // Mate within species
                                t_dad = GetIndividual(a_Parameters, a_RNG);

                                // The other parent should be a different one
                                // number of tries to find different parent
                                int t_tries = 1024;
                                if (!a_Parameters.AllowClones)
                                {
                                    while (((t_mom.GetID() == t_dad.GetID()) ||
                                            (t_mom.CompatibilityDistance(t_dad, a_Parameters) < COMPAT_EQUALITY_DELTA)) &&
                                           (t_tries--))
                                    {
                                        t_dad = GetIndividual(a_Parameters, a_RNG);
                                    }
                                }
                                else
                                {
                                    while (((t_mom.GetID() == t_dad.GetID())) && (t_tries--))
                                    {
                                        t_dad = GetIndividual(a_Parameters, a_RNG);
                                    }
                                }
                                t_interspecies = false;
                            }

                            // OK we have both mom and dad so mate them
                            // Choose randomly one of two types of crossover
                            if (a_RNG.RandFloat() < a_Parameters.MultipointCrossoverRate)
                            {
                                t_baby = t_mom.Mate(t_dad, false, t_interspecies, a_RNG, a_Parameters);
                            }
                            else
                            {
                                t_baby = t_mom.Mate(t_dad, true, t_interspecies, a_RNG, a_Parameters);
                            }

                            t_mated = true;
                        }
                            // don't mate - reproduce the mother asexually
                        else
                        {
                            t_baby = t_mom;
                            t_mated = false;
                        }
                    }

                    // Mutate the baby
                    if ((!t_mated) || (a_RNG.RandFloat() < a_Parameters.OverallMutationRate))
                    {
                        MutateGenome(t_baby_exists_in_pop, a_Pop, t_baby, a_Parameters, a_RNG);
                    }

                    // Check if this baby is already present somewhere in the offspring
                    // we don't want that
                    t_baby_exists_in_pop = false;
                    // Unless of course, we want clones to exist
                    if (!a_Parameters.AllowClones)
                    {
                        for (unsigned int i = 0; i < a_Pop.m_TempSpecies.size(); i++)
                        {
                            for (unsigned int j = 0; j < a_Pop.m_TempSpecies[i].m_Individuals.size(); j++)
                            {
                                if (
                                        (t_baby.CompatibilityDistance(a_Pop.m_TempSpecies[i].m_Individuals[j],
                                                                      a_Parameters) < COMPAT_EQUALITY_DELTA) // identical genome?
                                        )
                                {
                                    t_baby_exists_in_pop = true;
                                    break;
                                }
                            }
                        }
                    }

                    // In case we want to enforce always new individuals
                    if (a_Parameters.ArchiveEnforcement)
                    {
                        for (unsigned int i = 0; i < a_Pop.m_GenomeArchive.size(); i++)
                        {
                            if (
                                    (t_baby.CompatibilityDistance(a_Pop.m_GenomeArchive[i],
                                                                  a_Parameters) < COMPAT_EQUALITY_DELTA) // identical genome?
                                    )
                            {
                                t_baby_exists_in_pop = true;
                                break;
                            }
                        }
                    }
                }
                while (t_baby_exists_in_pop || (t_baby.FailsConstraints(a_Parameters))); // end do
            }

            // We have a new offspring now
            // give the offspring a new ID
            t_baby.SetID(a_Pop.GetNextGenomeID());
            a_Pop.IncrementNextGenomeID();

            // sort the baby's genes
            t_baby.SortGenes();

            // clear the baby's fitness
            t_baby.SetFitness(0);
            t_baby.SetAdjFitness(0);
            t_baby.SetOffspringAmount(0);

            t_baby.ResetEvaluated();

            // Archive the baby if needed
            if (a_Parameters.ArchiveEnforcement)
            {
                a_Pop.m_GenomeArchive.push_back(t_baby);
            }

            //////////////////////////////////
            // put the baby to its species  //
            //////////////////////////////////

            // before Reproduce() is invoked, it is assumed that a
            // clone of the population exists with the name of m_TempSpecies
            // we will store results there.
            // after all reproduction completes, the original species will be replaced back

            bool t_found = false;
            std::vector<Species>::iterator t_cur_species = a_Pop.m_TempSpecies.begin();

            // No species yet?
            if (t_cur_species == a_Pop.m_TempSpecies.end())
            {
                // create the first species and place the baby there
                a_Pop.m_TempSpecies.push_back(Species(t_baby, a_Pop.GetNextSpeciesID()));
                a_Pop.IncrementNextSpeciesID();
            }
            else
            {
                // try to find a compatible species
                Genome t_to_compare = t_cur_species->GetRepresentative();

                t_found = false;
                while ((t_cur_species != a_Pop.m_TempSpecies.end()) && (!t_found))
                {
                    if (t_baby.IsCompatibleWith(t_to_compare, a_Parameters))
                    {
                        // found a compatible species
                        t_cur_species->AddIndividual(t_baby);
                        t_found = true; // the search is over
                    }
                    else
                    {
                        // keep searching for a matching species
                        t_cur_species++;
                        if (t_cur_species != a_Pop.m_TempSpecies.end())
                        {
                            t_to_compare = t_cur_species->GetRepresentative();
                        }
                    }
                }

                // if couldn't find a match, make a new species
                if (!t_found)
                {
                    a_Pop.m_TempSpecies.push_back(Species(t_baby, a_Pop.GetNextSpeciesID()));
                    a_Pop.IncrementNextSpeciesID();
                }
            }
        }
    }


    ////////////
    // Real-time code
    void Species::CalculateAverageFitness()
    {
        double t_total_fitness = 0;
        int t_num_individuals = 0;

        // consider individuals that were evaluated only!
        for (unsigned int i = 0; i < m_Individuals.size(); i++)
        {
            if (m_Individuals[i].m_Evaluated)
            {
                t_total_fitness += m_Individuals[i].GetFitness();
                t_num_individuals++;
            }
        }

        if (t_num_individuals > 0)
            m_AverageFitness = t_total_fitness / static_cast<double>(t_num_individuals);
        else
            m_AverageFitness = 0;
    }


    Genome Species::ReproduceOne(Population &a_Pop, Parameters &a_Parameters, RNG &a_RNG)
    {
        Genome t_baby; // for storing the result

        //////////////////////////
        // Reproduction
        bool t_baby_exists_in_pop = false;
        bool t_baby_is_clone = false;

        // Spawn only one baby
        do // - while the baby turned invalid in some way
        {
            // this tells us if the baby is a result of mating
            bool t_mated = false;
        
            // There must be individuals there..
            ASSERT(NumIndividuals() > 0);
        
            // for a species of size 1 we can only mutate
            // NOTE: but does it make sense since we know this is the champ?
            if (NumIndividuals() == 1)
            {
                t_baby = GetIndividual(a_Parameters, a_RNG);
                t_mated = false;
            }
                // else we can mate
            else
            {
                Genome t_mom = GetIndividual(a_Parameters, a_RNG);
            
                // choose whether to mate at all
                // Do not allow crossover when in simplifying phase
                if ((a_RNG.RandFloat() < a_Parameters.CrossoverRate) && (a_Pop.GetSearchMode() != SIMPLIFYING))
                {
                    // get the father
                    Genome t_dad;
                    bool t_interspecies = false;
                
                    // There is a probability that the father may come from another species
                    if ((a_RNG.RandFloat() < a_Parameters.InterspeciesCrossoverRate) &&
                        (a_Pop.m_Species.size() > 1))
                    {
                        // Find different species (random one) // !!!!!!!!!!!!!!!!!
                        int t_diffspec = a_RNG.RandInt(0, static_cast<int>(a_Pop.m_Species.size() - 1));
                        t_dad = a_Pop.m_Species[t_diffspec].GetIndividual(a_Parameters, a_RNG);
                        t_interspecies = true;
                    }
                    else
                    {
                        // Mate within species
                        t_dad = GetIndividual(a_Parameters, a_RNG);
                    
                        // The other parent should be a different one
                        // number of tries to find different parent
                        int t_tries = 1024;
                        if (!a_Parameters.AllowClones)
                        {
                            while (((t_mom.GetID() == t_dad.GetID()) ||
                                    (t_mom.CompatibilityDistance(t_dad, a_Parameters) < COMPAT_EQUALITY_DELTA)) &&
                                   (t_tries--))
                            {
                                t_dad = GetIndividual(a_Parameters, a_RNG);
                            }
                        }
                        else
                        {
                            while (((t_mom.GetID() == t_dad.GetID())) && (t_tries--))
                            {
                                t_dad = GetIndividual(a_Parameters, a_RNG);
                            }
                        }
                        t_interspecies = false;
                    }
                
                    // OK we have both mom and dad so mate them
                    // Choose randomly one of two types of crossover
                    if (a_RNG.RandFloat() < a_Parameters.MultipointCrossoverRate)
                    {
                        t_baby = t_mom.Mate(t_dad, false, t_interspecies, a_RNG, a_Parameters);
                    }
                    else
                    {
                        t_baby = t_mom.Mate(t_dad, true, t_interspecies, a_RNG, a_Parameters);
                    }
                
                    t_mated = true;
                }
                    // don't mate - reproduce the mother asexually
                else
                {
                    t_baby = t_mom;
                    t_mated = false;
                }
            }
            
            // Mutate the baby
            t_baby_is_clone = false;
            if ((!t_mated) || (a_RNG.RandFloat() < a_Parameters.OverallMutationRate))
            {
                MutateGenome(t_baby_is_clone, a_Pop, t_baby, a_Parameters, a_RNG);
            }

            // Check if this baby is already present somewhere in the offspring
            // we don't want that
            t_baby_exists_in_pop = false;
            // Unless of course, we want clones to exist
            if (!a_Parameters.AllowClones)
            {
                for (unsigned int i = 0; i < a_Pop.m_Species.size(); i++)
                {
                    for (unsigned int j = 0; j < a_Pop.m_Species[i].m_Individuals.size(); j++)
                    {
                        if (
                                (t_baby.CompatibilityDistance(a_Pop.m_Species[i].m_Individuals[j],
                                                              a_Parameters) < COMPAT_EQUALITY_DELTA) // identical genome?
                                )
                        {
                            t_baby_exists_in_pop = true;
                            break;
                        }
                    }
                }
            }

            // In case we want to enforce always new individuals
            if (a_Parameters.ArchiveEnforcement)
            {
                for (unsigned int i = 0; i < a_Pop.m_GenomeArchive.size(); i++)
                {
                    if (
                            (t_baby.CompatibilityDistance(a_Pop.m_GenomeArchive[i],
                                                          a_Parameters) < COMPAT_EQUALITY_DELTA) // identical genome?
                            )
                    {
                        t_baby_exists_in_pop = true;
                        break;
                    }
                }
            }
        }
        while (t_baby_exists_in_pop || t_baby.FailsConstraints(a_Parameters)); // end do


        // We have a new offspring now
        // give the offspring a new ID
        t_baby.SetID(a_Pop.GetNextGenomeID());
        a_Pop.IncrementNextGenomeID();

        // sort the baby's genes
        t_baby.SortGenes();

        // clear the baby's fitness
        t_baby.SetFitness(0);
        t_baby.SetAdjFitness(0);
        t_baby.SetOffspringAmount(0);

        t_baby.ResetEvaluated();

        // In case of archiving, add the new baby to the archive
        if (a_Parameters.ArchiveEnforcement)
        {
            a_Pop.m_GenomeArchive.push_back(t_baby);
        }

        return std::move(t_baby);
    }


    // Mutates a genome
    void
    Species::MutateGenome(bool t_baby_is_clone, Population &a_Pop, Genome &t_baby, Parameters &a_Parameters, RNG &a_RNG)
    {
#if 0
        if ((a_RNG.RandFloat() < a_Parameters.MutateAddNeuronProb) && (a_Pop.GetSearchMode() != SIMPLIFYING))
        {
            t_baby.Mutate_AddNeuron(a_Pop.AccessInnovationDatabase(), a_Parameters, a_RNG);
        }
        else if ((a_RNG.RandFloat() < a_Parameters.MutateAddLinkProb) && (a_Pop.GetSearchMode() != SIMPLIFYING))
        {
            t_baby.Mutate_AddLink(a_Pop.AccessInnovationDatabase(), a_Parameters, a_RNG);
        }
        else if ((a_RNG.RandFloat() < a_Parameters.MutateRemSimpleNeuronProb) && (a_Pop.GetSearchMode() != COMPLEXIFYING))
        {
            t_baby.Mutate_RemoveSimpleNeuron(a_Pop.AccessInnovationDatabase(), a_RNG);
        }
        else if ((a_RNG.RandFloat() < a_Parameters.MutateRemLinkProb) && (a_Pop.GetSearchMode() != COMPLEXIFYING))
        {
            // Keep doing this mutation until it is sure that the baby will not
            // end up having dead ends or no links
            Genome t_saved_baby = t_baby;
            bool t_no_links = false, t_has_dead_ends = false;

            int t_tries = 128;
            do
            {
                t_tries--;
                if (t_tries <= 0)
                {
                    t_saved_baby = t_baby;
                    break; // give up
                }
    
                t_saved_baby = t_baby;
                t_saved_baby.Mutate_RemoveLink(a_RNG);
    
                t_no_links = t_has_dead_ends = false;
    
                if (t_saved_baby.NumLinks() == 0)
                    t_no_links = true;
    
                t_has_dead_ends = t_saved_baby.HasDeadEnds();
    
            }
            while (t_no_links || t_has_dead_ends);

            t_baby = t_saved_baby;
        }
        else
        {
            if (a_RNG.RandFloat() < a_Parameters.MutateNeuronActivationTypeProb)
                t_baby.Mutate_NeuronActivation_Type(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateWeightsProb)
                t_baby.Mutate_LinkWeights(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateActivationAProb)
                t_baby.Mutate_NeuronActivations_A(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateActivationBProb)
                t_baby.Mutate_NeuronActivations_B(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateNeuronTimeConstantsProb)
                t_baby.Mutate_NeuronTimeConstants(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateNeuronBiasesProb)
                t_baby.Mutate_NeuronBiases(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateNeuronTraitsProb)
                t_baby.Mutate_NeuronTraits(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateLinkTraitsProb)
                t_baby.Mutate_LinkTraits(a_Parameters, a_RNG);
    
            if (a_RNG.RandFloat() < a_Parameters.MutateGenomeTraitsProb)
                t_baby.Mutate_GenomeTraits(a_Parameters, a_RNG);
        }
#else
        // We will perform roulette wheel selection to choose the type of mutation and will mutate the baby
        // This method guarantees that the baby will be mutated at least with one mutation
        enum MutationTypes
        {
            ADD_NODE = 0, ADD_LINK, REMOVE_NODE, REMOVE_LINK, CHANGE_ACTIVATION_FUNCTION,
            MUTATE_WEIGHTS, MUTATE_ACTIVATION_A, MUTATE_ACTIVATION_B, MUTATE_TIMECONSTS, MUTATE_BIASES,
            MUTATE_NEURON_TRAITS, MUTATE_LINK_TRAITS, MUTATE_GENOME_TRAITS,

            MutationTypesCount // should go after last type id
        };
        
        std::vector<double> t_mut_probs(MutationTypesCount);
        
        t_mut_probs[ADD_NODE] = a_Parameters.MutateAddNeuronProb;
        t_mut_probs[ADD_LINK] = (a_Parameters.MutateAddLinkProb);
        t_mut_probs[REMOVE_NODE] = a_Parameters.MutateRemSimpleNeuronProb;
        t_mut_probs[REMOVE_LINK] = a_Parameters.MutateRemLinkProb;
        t_mut_probs[CHANGE_ACTIVATION_FUNCTION] = a_Parameters.MutateNeuronActivationTypeProb;
        t_mut_probs[MUTATE_WEIGHTS] = a_Parameters.MutateWeightsProb;
        t_mut_probs[MUTATE_ACTIVATION_A] = a_Parameters.MutateActivationAProb;
        t_mut_probs[MUTATE_ACTIVATION_B] = a_Parameters.MutateActivationBProb;
        t_mut_probs[MUTATE_TIMECONSTS] = a_Parameters.MutateNeuronTimeConstantsProb;
        t_mut_probs[MUTATE_BIASES] = a_Parameters.MutateNeuronBiasesProb;
        t_mut_probs[MUTATE_NEURON_TRAITS] =  a_Parameters.MutateNeuronTraitsProb;
        t_mut_probs[MUTATE_LINK_TRAITS] =  a_Parameters.MutateLinkTraitsProb;
        t_mut_probs[MUTATE_GENOME_TRAITS] =  a_Parameters.MutateGenomeTraitsProb;

        // Special consideration for phased searching - do not allow certain mutations depending on the search mode
        // also don't use additive mutations if we just want to get rid of the clones
        if ((a_Pop.GetSearchMode() == SIMPLIFYING) || t_baby_is_clone)
        {
            t_mut_probs[ADD_NODE] = 0; // add node
            t_mut_probs[ADD_LINK] = 0; // add link
        }
        if ((a_Pop.GetSearchMode() == COMPLEXIFYING) || t_baby_is_clone)
        {
            t_mut_probs[REMOVE_NODE] = 0; // rem node
            t_mut_probs[REMOVE_LINK] = 0; // rem link
        }
    
        bool t_mutation_success = false;
    
        // repeat until successful
        while (t_mutation_success == false)
        {
            int ChosenMutation = a_RNG.Roulette(t_mut_probs);
        
            // Now mutate based on the choice
            switch (ChosenMutation)
            {
                case ADD_NODE:
                    t_mutation_success = t_baby.Mutate_AddNeuron(a_Pop.AccessInnovationDatabase(), a_Parameters, a_RNG);
                    break;
            
                case ADD_LINK:
                    t_mutation_success = t_baby.Mutate_AddLink(a_Pop.AccessInnovationDatabase(), a_Parameters, a_RNG);
                    break;
            
                case REMOVE_NODE:
                    t_mutation_success = t_baby.Mutate_RemoveSimpleNeuron(a_Pop.AccessInnovationDatabase(), a_RNG);
                    break;
            
                case REMOVE_LINK:
                {
                    // Keep doing this mutation until it is sure that the baby will not
                    // end up having dead ends or no links
                    Genome t_saved_baby = t_baby;
                    bool t_no_links = false, t_has_dead_ends = false;
                
                    int t_tries = 128;
                    do
                    {
                        t_tries--;
                        if (t_tries <= 0)
                        {
                            t_saved_baby = t_baby;
                            break; // give up
                        }
                    
                        t_saved_baby = t_baby;
                        t_mutation_success = t_saved_baby.Mutate_RemoveLink(a_RNG);
                    
                        t_no_links = t_has_dead_ends = false;
                    
                        if (t_saved_baby.NumLinks() == 0)
                            t_no_links = true;
                    
                        t_has_dead_ends = t_saved_baby.HasDeadEnds();
                    
                    }
                    while (t_no_links || t_has_dead_ends);
                
                    t_baby = t_saved_baby;
                }
                    break;
            
                case CHANGE_ACTIVATION_FUNCTION:
                    t_mutation_success = t_baby.Mutate_NeuronActivation_Type(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_WEIGHTS:
                    t_mutation_success = t_baby.Mutate_LinkWeights(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_ACTIVATION_A:
                    t_mutation_success = t_baby.Mutate_NeuronActivations_A(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_ACTIVATION_B:
                    t_mutation_success = t_baby.Mutate_NeuronActivations_B(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_TIMECONSTS:
                    t_mutation_success = t_baby.Mutate_NeuronTimeConstants(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_BIASES:
                    t_mutation_success = t_baby.Mutate_NeuronBiases(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_NEURON_TRAITS:
                    t_mutation_success = t_baby.Mutate_NeuronTraits(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_LINK_TRAITS:
                    t_mutation_success = t_baby.Mutate_LinkTraits(a_Parameters, a_RNG);
                    break;
            
                case MUTATE_GENOME_TRAITS:
                    t_mutation_success = t_baby.Mutate_GenomeTraits(a_Parameters, a_RNG);
                    break;
            
                default:
                    t_mutation_success = false;
                    break;
            }
        }
#endif
    }
    
} // namespace NEAT

