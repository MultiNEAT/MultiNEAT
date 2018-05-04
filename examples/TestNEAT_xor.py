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

max_runs = 10
max_generations = 150


class XorNeatRunner(NEAT.Runner):
    def __init__(self, *args, **kwargs):
        NEAT.Runner.__init__(self, experiment = XorExperiment(depth=5), *args, **kwargs)

    def create_seed_population(self):
        seed_genome = NEAT.Genome(0, 3, 0, 1, False, NEAT.ActivationFunction.UNSIGNED_SIGMOID,
                            NEAT.ActivationFunction.UNSIGNED_SIGMOID, 0, params, 0)

        should_randomize_weights = True
        random_weights_magnitude = 1.0
        # random_seed = 1234
        random_seed = int(time.clock()*100)

        return NEAT.Population(seed_genome, params, should_randomize_weights, random_weights_magnitude, random_seed)

    def build_phenotype(self, genome):
        net = NEAT.NeuralNetwork()
        genome.BuildPhenotype(net)
        return net

    def evaluate_fitness(self, genome):
        net = self.build_phenotype(genome)
        return self.experiment.fitness(net)

    def is_solved(self, best_fitness):
        return self.experiment.is_solved(best_fitness)

gens = []
for run in range(max_runs):

    runner = XorNeatRunner()
    runner.run(max_generations = max_generations)

    gens += [runner.generations_to_solve]

    net = NEAT.NeuralNetwork()
    runner.population.GetBestGenome().BuildPhenotype(net)

    print('Run: {}/{}'.format(run, max_runs - 1),
        'Generations to solve XOR:', runner.generations_to_solve,
        '| in %3.2f ms per gen, %3.4f s total' % (runner.time_per_generation, runner.total_time), 
        "complexity ({}, {})".format(net.NumHiddenNeurons(), net.NumConnections()))

avg_gens = sum(gens) / len(gens)

print('All:', gens)
print('Average:', avg_gens)
