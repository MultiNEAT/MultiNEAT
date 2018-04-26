#ifndef _GENES_H
#define _GENES_H

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

/////////////////////////////////////////////////////////////////
// File:        Genes.h
// Description: Definitions for the Neuron and Link gene classes.
/////////////////////////////////////////////////////////////////

#ifdef USE_BOOST_PYTHON

#include <boost/python.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace py = boost::python;

#endif

#include <iostream>
#include <vector>
#include <map>
#include "Parameters.h"
#include "Traits.h"
#include "Random.h"
#include "Utils.h"


namespace NEAT
{


//////////////////////////////////////////////
// Enumeration for all available neuron types
//////////////////////////////////////////////
    enum NeuronType
    {
        NONE = 0,
        INPUT,
        BIAS,
        HIDDEN,
        OUTPUT
    };


//////////////////////////////////////////////////////////
// Enumeration for all possible activation function types
//////////////////////////////////////////////////////////
    enum ActivationFunction
    {
        SIGNED_SIGMOID = 0,   // Sigmoid function   (default) (blurred cutting plane)
        UNSIGNED_SIGMOID,
        TANH,
        TANH_CUBIC,
        SIGNED_STEP,          // Treshold (0 or 1)  (cutting plane)
        UNSIGNED_STEP,
        SIGNED_GAUSS,         // Gaussian           (symettry)
        UNSIGNED_GAUSS,
        ABS,                  // Absolute value |x| (another symettry)
        SIGNED_SINE,          // Sine wave          (smooth repetition)
        UNSIGNED_SINE,
        LINEAR,               // Linear f(x)=x      (combining coordinate frames only)

        RELU,                 // Rectifiers
        SOFTPLUS

    };

    //////////////////////////////////
    // Base Gene class
    //////////////////////////////////
    class Gene
    {
    public:
        // Arbitrary traits
        std::map<std::string, Trait> m_Traits;

        Gene &operator=(const Gene &a_g);

        // Randomize based on parameters
        void InitTraits(const std::map<std::string, TraitParameters> &tp, RNG &a_RNG);

        // Traits are merged with this other parent
        void MateTraits(const std::map<std::string, Trait> &t, RNG &a_RNG);

        // Traits are mutated according to parameters
        bool MutateTraits(const std::map<std::string, TraitParameters> &tp, RNG &a_RNG);

        // Compute and return distances between each matching pair of traits
        std::map<std::string, double> GetTraitDistances(const std::map<std::string, Trait> &other) const;
    };


    //////////////////////////////////
    // This class defines a link gene
    //////////////////////////////////
    class LinkGene : public Gene
    {
        /////////////////////
        // Members
        /////////////////////

    public:

        // These variables are initialized once and cannot be changed
        // anymore

        // The IDs of the neurons that this link connects
        int m_FromNeuronID, m_ToNeuronID;

        // The link's innovation ID
        int m_InnovationID;

        // This variable is modified during evolution
        // The weight of the connection
        double m_Weight;

        // Is it recurrent?
        bool m_IsRecurrent;

    public:

#ifdef USE_BOOST_PYTHON
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & m_FromNeuronID;
            ar & m_ToNeuronID;
            ar & m_InnovationID;
            ar & m_IsRecurrent;
            ar & m_Weight;

            // the traits too, TODO
            //ar & m_Traits;
        }
#endif

        double GetWeight() const;

        void SetWeight(const double a_Weight);

        ////////////////
        // Constructors
        ////////////////
        LinkGene() = default;

        // Copy
        LinkGene(const LinkGene &a_g) = default;
        LinkGene &operator=(const LinkGene &a_g) = default;

        // Move
        LinkGene(LinkGene &&a_g) = default;
        LinkGene &operator=(LinkGene &&a_g) = default;

        LinkGene(int a_InID, int a_OutID, int a_InnovID, double a_Wgt, bool a_Recurrent = false);


        //////////////
        // Methods
        //////////////

        // Access to static (const) variables
        int FromNeuronID() const;

        int ToNeuronID() const;

        int InnovationID() const;

        bool IsRecurrent() const;

        bool IsLoopedRecurrent() const;

        //overload '<', '>', '!=' and '==' used for sorting and comparison (we use the innovation ID as the criteria)
        friend bool operator<(const LinkGene &a_lhs, const LinkGene &a_rhs);

        friend bool operator>(const LinkGene &a_lhs, const LinkGene &a_rhs);

        friend bool operator!=(const LinkGene &a_lhs, const LinkGene &a_rhs);

        friend bool operator==(const LinkGene &a_lhs, const LinkGene &a_rhs);
    };


////////////////////////////////////
// This class defines a neuron gene
////////////////////////////////////
    class NeuronGene : public Gene
    {
        /////////////////////
        // Members
        /////////////////////

    public:
        // These variables are initialized once and cannot be changed
        // anymore

        // Its unique identification number
        int m_ID;

        // Its type and role in the network
        NeuronType m_Type;

    public:
        // These variables are modified during evolution
        // Safe to access directly

        // useful for displaying the genome
        int x, y;
        // Position (depth) within the network
        double m_SplitY;


        /////////////////////////////////////////////////////////
        // Any additional properties of the neuron
        // should be added here. This may include
        // time constant & bias for leaky integrators,
        // activation function type,
        // activation function slope (or maybe other properties),
        // etc...
        /////////////////////////////////////////////////////////

        // Additional parameters associated with the
        // neuron's activation function.
        // The current activation function may not use
        // any of them anyway.
        // A is usually used to alter the function's slope with a scalar
        // B is usually used to force a bias to the neuron
        // -------------------
        // Sigmoid : using A, B (slope, shift)
        // Step    : using B    (shift)
        // Gauss   : using A, B (slope, shift))
        // Abs     : using B    (shift)
        // Sine    : using A    (frequency, phase)
        // Square  : using A, B (high phase lenght, low phase length)
        // Linear  : using B    (shift)
        double m_A, m_B;

        // Time constant value used when
        // the neuron is activating in leaky integrator mode
        double m_TimeConstant;

        // Bias value used when the neuron is activating in
        // leaky integrator mode
        double m_Bias;

        // The type of activation function the neuron has
        ActivationFunction m_ActFunction;

#ifdef USE_BOOST_PYTHON
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & m_ID;
            ar & m_Type;
            ar & m_A;
            ar & m_B;
            ar & m_TimeConstant;
            ar & m_Bias;
            ar & x;
            ar & y;
            ar & m_ActFunction;
            ar & m_SplitY;

            // TODO the traits also
            //ar & m_Traits;
        }
#endif

        ////////////////
        // Constructors
        ////////////////
        NeuronGene() = default;

        // Copy
        NeuronGene(const NeuronGene &a_g) = default;
        NeuronGene &operator=(const NeuronGene &a_g) = default;

        // Move
        NeuronGene(NeuronGene &&a_g) = default;
        NeuronGene &operator=(NeuronGene &&a_g) = default;


        friend bool operator==(const NeuronGene &a_lhs, const NeuronGene &a_rhs);

        NeuronGene(NeuronType a_type, int a_id, double a_splity);



        //////////////
        // Methods
        //////////////

        // Accessing static (const) variables
        int ID() const
        {
            return m_ID;
        }

        NeuronType Type() const
        {
            return m_Type;
        }

        double SplitY() const
        {
            return m_SplitY;
        }

        // Initializing
        void Init(double a_A, double a_B, double a_TimeConstant, double a_Bias, ActivationFunction a_ActFunc)
        {
            m_A = a_A;
            m_B = a_B;
            m_TimeConstant = a_TimeConstant;
            m_Bias = a_Bias;
            m_ActFunction = a_ActFunc;
        }
    };


} // namespace NEAT

#endif
