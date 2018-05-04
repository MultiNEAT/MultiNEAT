//
// Created by Anton Matosov on 3/25/18.
//


#include "Genes.h"

namespace NEAT
{

NEAT::Gene &NEAT::Gene::operator=(const Gene &a_g)
{
    if (this != &a_g) {
        m_Traits = a_g.m_Traits;
    }

    return *this;
}

void Gene::InitTraits(const std::map<std::string, TraitParameters> &tp, RNG &a_RNG)
{
    for (auto it = tp.begin(); it != tp.end(); it++) {
        // Check what kind of type is this and create such trait
        TraitType t;

        if (it->second.type == "int") {
            IntTraitParameters itp = bs::get<IntTraitParameters>(it->second.m_Details);
            t = a_RNG.RandInt(itp.min, itp.max);
        }
        if (it->second.type == "float") {
            FloatTraitParameters itp = bs::get<FloatTraitParameters>(it->second.m_Details);
            double x = a_RNG.RandFloat();
            Scale(x, 0, 1, itp.min, itp.max);
            t = x;
        }
        if (it->second.type == "str") {
            StringTraitParameters itp = bs::get<StringTraitParameters>(it->second.m_Details);
            std::vector<double> probs = itp.probs;
            if (itp.set.size() == 0) {
                throw std::runtime_error("Empty set of string traits");
            }
            probs.resize(itp.set.size());

            int idx = a_RNG.Roulette(probs);
            t = itp.set[idx];
        }
        if (it->second.type == "intset") {
            IntSetTraitParameters itp = bs::get<IntSetTraitParameters>(it->second.m_Details);
            std::vector<double> probs = itp.probs;
            if (itp.set.size() == 0) {
                throw std::runtime_error("Empty set of int traits");
            }
            probs.resize(itp.set.size());

            int idx = a_RNG.Roulette(probs);
            t = itp.set[idx];
        }
        if (it->second.type == "floatset") {
            FloatSetTraitParameters itp = bs::get<FloatSetTraitParameters>(it->second.m_Details);
            std::vector<double> probs = itp.probs;
            if (itp.set.size() == 0) {
                throw std::runtime_error("Empty set of float traits");
            }
            probs.resize(itp.set.size());

            int idx = a_RNG.Roulette(probs);
            t = itp.set[idx];
        }
#ifdef USE_BOOST_PYTHON
        if (it->second.type == "pyobject") {
            py::object itp = bs::get<py::object>(it->second.m_Details);
            t = itp(); // details is a function that returns a random instance of the trait
        }
#endif
        Trait tr;
        tr.value = t;
        tr.dep_key = it->second.dep_key;
        tr.dep_values = it->second.dep_values;
        // todo check for invalid dep_values types here
        m_Traits[it->first] = tr;
    }
}

void Gene::MateTraits(const std::map<std::string, Trait> &t, RNG &a_RNG)
{
    for (auto it = t.begin(); it != t.end(); it++) {
        TraitType mine = m_Traits[it->first].value;
        TraitType yours = it->second.value;

        if (!(mine.type() == yours.type())) {
            throw std::runtime_error("Types of traits doesn't match");
        }

        // if generic python object, forward all processing to its method
#ifdef USE_BOOST_PYTHON
        if (mine.type() == typeid(py::object)) {
            // call mating function
            m_Traits[it->first].value = bs::get<py::object>(mine).attr("mate")(bs::get<py::object>(yours));
        } 
        else
#endif
        {

            if (a_RNG.RandFloat() < 0.5) // pick either one
            {
                m_Traits[it->first].value = (a_RNG.RandFloat() < 0.5) ? mine : yours;
            } else {
                // try to average
                if (mine.type() == typeid(int)) {
                    int m1 = bs::get<int>(mine);
                    int m2 = bs::get<int>(yours);
                    m_Traits[it->first].value = (m1 + m2) / 2;
                }

                if (mine.type() == typeid(double)) {
                    double m1 = bs::get<double>(mine);
                    double m2 = bs::get<double>(yours);
                    m_Traits[it->first].value = (m1 + m2) / 2.0;
                }

                if (mine.type() == typeid(std::string)) {
                    // strings are always either-or
                    m_Traits[it->first].value = (a_RNG.RandFloat() < 0.5) ? mine : yours;
                }

                if (mine.type() == typeid(intsetelement)) {
                    // int sets are always either-or
                    m_Traits[it->first].value = (a_RNG.RandFloat() < 0.5) ? mine : yours;
                }

                if (mine.type() == typeid(floatsetelement)) {
                    // float sets are always either-or
                    m_Traits[it->first].value = (a_RNG.RandFloat() < 0.5) ? mine : yours;
                }
            }
        }
    }
}

bool Gene::MutateTraits(const std::map<std::string, TraitParameters> &tp, RNG &a_RNG)
{
    bool did_mutate = false;
    for (auto it = tp.begin(); it != tp.end(); it++) {
        // Check what kind of type is this and modify it
        TraitType t;

        // only mutate the trait if it's enabled
        bool doit = false;
        if (it->second.dep_key != "") {
            // there is such trait..
            if (m_Traits.count(it->second.dep_key) != 0) {
                // and it matches any of the right values?
                for (int ix = 0; ix < it->second.dep_values.size(); ix++) {
                    if (m_Traits[it->second.dep_key].value == it->second.dep_values[ix]) {
                        doit = true;
                        break;
                    }
                }
            }
        } else {
            doit = true;
        }

        if (doit) {
            if (it->second.type == "int") {
                IntTraitParameters itp = bs::get<IntTraitParameters>(it->second.m_Details);

                // Mutate?
                if (a_RNG.RandFloat() < it->second.m_MutationProb) {
                    // determine type of mutation - modify or replace, according to parameters
                    if (a_RNG.RandFloat() < itp.mut_replace_prob) {
                        // replace
                        int val = 0;
                        int cur = bs::get<int>(m_Traits[it->first].value);
                        val = a_RNG.RandInt(itp.min, itp.max);
                        m_Traits[it->first].value = val;
                        if (cur != val)
                            did_mutate = true;
                    } else {
                        // modify
                        int val = bs::get<int>(m_Traits[it->first].value);
                        int cur = val;
                        val += a_RNG.RandInt(-itp.mut_power, itp.mut_power);
                        Clamp(val, itp.min, itp.max);
                        m_Traits[it->first].value = val;
                        if (cur != val)
                            did_mutate = true;
                    }
                }
            }
            if (it->second.type == "float") {
                FloatTraitParameters itp = bs::get<FloatTraitParameters>(it->second.m_Details);

                // Mutate?
                if (a_RNG.RandFloat() < it->second.m_MutationProb) {
                    // determine type of mutation - modify or replace, according to parameters
                    if (a_RNG.RandFloat() < itp.mut_replace_prob) {
                        // replace
                        double val = 0;
                        double cur = bs::get<double>(m_Traits[it->first].value);
                        val = a_RNG.RandFloat();
                        Scale(val, 0, 1, itp.min, itp.max);
                        m_Traits[it->first].value = val;
                        if (cur != val)
                            did_mutate = true;
                    } else {
                        // modify
                        double val = bs::get<double>(m_Traits[it->first].value);
                        double cur = val;
                        val += a_RNG.RandFloatSigned() * itp.mut_power;
                        Clamp(val, itp.min, itp.max);
                        m_Traits[it->first].value = val;
                        if (cur != val)
                            did_mutate = true;
                    }
                }
            }
            if (it->second.type == "str") {
                StringTraitParameters itp = bs::get<StringTraitParameters>(it->second.m_Details);
                std::vector<double> probs = itp.probs;
                probs.resize(itp.set.size());

                int idx = a_RNG.Roulette(probs);
                std::string cur = bs::get<std::string>(m_Traits[it->first].value);

                // now choose the new idx from the set
                m_Traits[it->first].value = itp.set[idx];
                if (cur != itp.set[idx])
                    did_mutate = true;
            }
            if (it->second.type == "intset") {
                IntSetTraitParameters itp = bs::get<IntSetTraitParameters>(it->second.m_Details);
                std::vector<double> probs = itp.probs;
                probs.resize(itp.set.size());

                int idx = a_RNG.Roulette(probs);
                intsetelement cur = bs::get<intsetelement>(m_Traits[it->first].value);

                // now choose the new idx from the set
                m_Traits[it->first].value = itp.set[idx];
                if (cur.value != itp.set[idx].value)
                    did_mutate = true;
            }
            if (it->second.type == "floatset") {
                FloatSetTraitParameters itp = bs::get<FloatSetTraitParameters>(it->second.m_Details);
                std::vector<double> probs = itp.probs;
                probs.resize(itp.set.size());

                int idx = a_RNG.Roulette(probs);
                floatsetelement cur = bs::get<floatsetelement>(m_Traits[it->first].value);

                // now choose the new idx from the set
                m_Traits[it->first].value = itp.set[idx];
                if (cur.value != itp.set[idx].value)
                    did_mutate = true;
            }
#ifdef USE_BOOST_PYTHON
            if (it->second.type == "pyobject") {
                m_Traits[it->first].value = bs::get<py::object>(m_Traits[it->first].value).attr("mutate")();
                did_mutate = true;
            }
#endif
        }
    }

    return did_mutate;
}

std::map<std::string, double> Gene::GetTraitDistances(const std::map<std::string, Trait> &other) const
{
    std::map<std::string, double> dist;
    for (auto it = other.begin(); it != other.end(); it++) {
        TraitType mine = m_Traits.at(it->first).value;
        TraitType yours = it->second.value;

        if (!(mine.type() == yours.type())) {
            throw std::runtime_error("Types of traits don't match");
        }

        // only do it if the trait if it's enabled
        // todo: not sure about the distance, think more about it
        bool doit = false;
        if (it->second.dep_key != "") {
            // there is such trait..
            if (m_Traits.count(it->second.dep_key) != 0) {
                // and it has the right value?
                // also the other genome has to have the trait turned on
                for (int ix = 0; ix < it->second.dep_values.size(); ix++) {
                    if ((m_Traits.at(it->second.dep_key).value == it->second.dep_values[ix]) &&
                        (other.at(it->second.dep_key).value == it->second.dep_values[ix])) {
                        doit = true;
                        break;
                    }
                }
            }
        } else {
            doit = true;
        }

        if (doit) {
            if (mine.type() == typeid(int)) {
                // distance between ints - calculate directly
                dist[it->first] = abs(bs::get<int>(mine) - bs::get<int>(yours));
            }
            if (mine.type() == typeid(double)) {
                // distance between floats - calculate directly
                dist[it->first] = abs(bs::get<double>(mine) - bs::get<double>(yours));
            }
            if (mine.type() == typeid(std::string)) {
                // distance between strings - matching is 0, non-matching is 1
                if (bs::get<std::string>(mine) == bs::get<std::string>(yours)) {
                    dist[it->first] = 0.0;
                } else {
                    dist[it->first] = 1.0;
                }
            }
            if (mine.type() == typeid(intsetelement)) {
                // distance between ints - calculate directly
                dist[it->first] = abs((bs::get<intsetelement>(mine)).value - (bs::get<intsetelement>(yours)).value);
            }
            if (mine.type() == typeid(floatsetelement)) {
                // distance between floats - calculate directly
                dist[it->first] = abs(
                    (bs::get<floatsetelement>(mine)).value - (bs::get<floatsetelement>(yours)).value);
            }
#ifdef USE_BOOST_PYTHON
            if (mine.type() == typeid(py::object)) {
                // distance between objects - calculate via method
                dist[it->first] = py::extract<double>(
                    bs::get<py::object>(mine).attr("distance_to")(bs::get<py::object>(yours)));
            }
#endif
        }
    }

    return dist;
}

double LinkGene::GetWeight() const
{
    return m_Weight;
}

void LinkGene::SetWeight(const double a_Weight)
{
    m_Weight = a_Weight;
}

LinkGene::LinkGene(int a_InID, int a_OutID, int a_InnovID, double a_Wgt, bool a_Recurrent)
{
    m_FromNeuronID = a_InID;
    m_ToNeuronID = a_OutID;
    m_InnovationID = a_InnovID;

    m_Weight = a_Wgt;
    m_IsRecurrent = a_Recurrent;
}

int LinkGene::FromNeuronID() const
{
    return m_FromNeuronID;
}

int LinkGene::ToNeuronID() const
{
    return m_ToNeuronID;
}

int LinkGene::InnovationID() const
{
    return m_InnovationID;
}

bool LinkGene::IsRecurrent() const
{
    return m_IsRecurrent;
}

bool LinkGene::IsLoopedRecurrent() const
{
    return m_FromNeuronID == m_ToNeuronID;
}

bool operator<(const LinkGene &a_lhs, const LinkGene &a_rhs)
{
    return (a_lhs.m_InnovationID < a_rhs.m_InnovationID);
}

bool operator>(const LinkGene &a_lhs, const LinkGene &a_rhs)
{
    return (a_lhs.m_InnovationID > a_rhs.m_InnovationID);
}

bool operator!=(const LinkGene &a_lhs, const LinkGene &a_rhs)
{
    return (a_lhs.m_InnovationID != a_rhs.m_InnovationID);
}

bool operator==(const LinkGene &a_lhs, const LinkGene &a_rhs)
{
    return (a_lhs.m_InnovationID == a_rhs.m_InnovationID);
}

bool operator==(const NeuronGene &a_lhs, const NeuronGene &a_rhs)
{
    return (a_lhs.m_ID == a_rhs.m_ID) &&
           (a_lhs.m_Type == a_rhs.m_Type) &&
           (a_lhs.m_SplitY == a_rhs.m_SplitY) &&
           (a_lhs.m_A == a_rhs.m_A) &&
           (a_lhs.m_B == a_rhs.m_B) &&
           (a_lhs.m_TimeConstant == a_rhs.m_TimeConstant) &&
           (a_lhs.m_Bias == a_rhs.m_Bias) &&
           (a_lhs.m_ActFunction == a_rhs.m_ActFunction);
}

NeuronGene::NeuronGene(NeuronType a_type, int a_id, double a_splity)
{
    m_ID = a_id;
    m_Type = a_type;
    m_SplitY = a_splity;

    // Initialize the node specific parameters
    m_A = 0.0f;
    m_B = 0.0f;
    m_TimeConstant = 0.0f;
    m_Bias = 0.0f;
    m_ActFunction = UNSIGNED_SIGMOID;

    x = 0;
    y = 0;
}
}