#!/usr/bin/python3
from __future__ import print_function

import os
import sys
#sys.path.insert(0, '/home/peter/code/projects/MultiNEAT') # duh
import time
import random as rnd
import numpy as np
import cv2
import pickle as pickle
import MultiNEAT as NEAT
from MultiNEAT import EvaluateGenomeList_Serial, EvaluateGenomeList_Parallel
from MultiNEAT import GetGenomeList, ZipFitness
from XorExperiment import *
from concurrent.futures import ProcessPoolExecutor, as_completed


def evaluate(genome):
    net = NEAT.NeuralNetwork()
    genome.BuildPhenotype(net)

    experiment = XorExperiment(depth=5)

    return experiment.fitness(net)


params = NEAT.Parameters()
params.PopulationSize = 100
params.DynamicCompatibility = True
params.NormalizeGenomeSize = True
params.WeightDiffCoeff = 0.1
params.CompatTreshold = 2.0
params.YoungAgeTreshold = 15
params.SpeciesMaxStagnation = 15
params.OldAgeTreshold = 35
params.MinSpecies = 2
params.MaxSpecies = 10
params.RouletteWheelSelection = False
params.RecurrentProb = 0.0
params.OverallMutationRate = 1.0

params.ArchiveEnforcement = False

params.MutateWeightsProb = 0.05

params.WeightMutationMaxPower = 0.5
params.WeightReplacementMaxPower = 8.0
params.MutateWeightsSevereProb = 0.0
params.WeightMutationRate = 0.25
params.WeightReplacementRate = 0.9

params.MaxWeight = 8

params.MutateAddNeuronProb = 0.001
params.MutateAddLinkProb = 0.3
params.MutateRemLinkProb = 0.0

params.MinActivationA = 4.9
params.MaxActivationA = 4.9

params.ActivationFunction_SignedSigmoid_Prob = 0.0
params.ActivationFunction_UnsignedSigmoid_Prob = 1.0
params.ActivationFunction_Tanh_Prob = 0.0
params.ActivationFunction_SignedStep_Prob = 0.0

params.CrossoverRate = 0.0
params.MultipointCrossoverRate = 0.0
params.SurvivalRate = 0.2

params.MutateNeuronTraitsProb = 0
params.MutateLinkTraitsProb = 0

params.AllowLoops = True
params.AllowClones = True

max_runs = 100
max_generations = 150

def getbest(run_index):

    seed_genome = NEAT.Genome(0, 3, 0, 1, False, NEAT.ActivationFunction.UNSIGNED_SIGMOID,
                    NEAT.ActivationFunction.UNSIGNED_SIGMOID, 0, params, 0)

    shouldRandomizeWeights = True
    randomWeightsMagnitude = 1.0
    # randomSeed = 1234
    randomSeed = int(time.clock()*100)
    pop = NEAT.Population(seed_genome, params, shouldRandomizeWeights, randomWeightsMagnitude, randomSeed)

    generations_to_solve = "<N/A>"
    for generation in range(max_generations):
        fitness_list = NEAT.EvaluateSerial(pop, evaluate, display=False)
        
        best = max(fitness_list)
        if best > 15.0:
            generations_to_solve = generation
            break



    net = NEAT.NeuralNetwork()
    pop.GetBestGenome().BuildPhenotype(net)

    # if net.NumHiddenNeurons() == 0:
    #     img = NEAT.viz.Draw(net)
    #     cv2.imshow("solved with 0 nodes", img)
    #     cv2.waitKey(1)
    
    return generations_to_solve, net.NumHiddenNeurons(), net.NumConnections()


gens = []
for run in range(max_runs):
    curtime = time.time()

    gen, nodes, connections = getbest(run)
    gens += [gen]

    elapsed = time.time() - curtime
    elapsedPerGen = (elapsed / gen) * 1000
    print('Run: {}/{}'.format(run, max_runs - 1), 'Generations to solve XOR:', gen, '| in %3.2f ms per gen, %3.4f s total' % (elapsedPerGen, elapsed), "complexity ({}, {})".format(nodes, connections))
avg_gens = sum(gens) / len(gens)

print('All:', gens)
print('Average:', avg_gens)
