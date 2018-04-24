
import gym
import time

import MultiNEAT as NEAT
import MultiNEAT.viz as viz
import random as rnd
import pickle
import numpy as np
import cv2

from MultiNEAT import EvaluateGenomeList_Serial, EvaluateGenomeList_Parallel, GetGenomeList, ZipFitness

rng = NEAT.RNG()
rng.TimeSeed()

params = NEAT.Parameters()
params.PopulationSize = 150
params.DynamicCompatibility = True
params.WeightDiffCoeff = 1.0
params.CompatTreshold = 2.0
params.YoungAgeTreshold = 15
params.SpeciesMaxStagnation = 15
params.OldAgeTreshold = 35
params.MinSpecies = 2
params.MaxSpecies = 4
params.RouletteWheelSelection = False
params.Elitism = True
params.RecurrentProb = 0.15
params.OverallMutationRate = 0.2

params.MutateWeightsProb = 0.8
params.MutateNeuronTimeConstantsProb = 0.1
params.MutateNeuronBiasesProb = 0.1

params.WeightMutationMaxPower = 0.5
params.WeightReplacementMaxPower = 1.0
params.MutateWeightsSevereProb = 0.5
params.WeightMutationRate = 0.25

params.TimeConstantMutationMaxPower = 0.1
params.BiasMutationMaxPower = params.WeightMutationMaxPower

params.MaxWeight = 8

params.MutateAddNeuronProb = 0.1
params.MutateAddLinkProb = 0.2
params.MutateRemLinkProb = 0.0

params.MinActivationA  = 1.0
params.MaxActivationA  = 6.0

params.MinNeuronTimeConstant = 0.04
params.MaxNeuronTimeConstant = 0.24

params.MinNeuronBias = -params.MaxWeight
params.MaxNeuronBias = params.MaxWeight

params.ActivationFunction_SignedSigmoid_Prob = 0.0
params.ActivationFunction_UnsignedSigmoid_Prob = 0.0
params.ActivationFunction_Tanh_Prob = 1.0
params.ActivationFunction_SignedStep_Prob = 0.0
params.ActivationFunction_Linear_Prob = 0.0

params.CrossoverRate = 0.75  # mutate only 0.25
params.MultipointCrossoverRate = 0.4
params.SurvivalRate = 0.2

params.MutateNeuronTraitsProb = 0
params.MutateLinkTraitsProb = 0


trials = 15
max_generations = 20
ticks_per_trial = 300
render_during_training = False

### Debug
# trials = 1
# max_generations = 5
# ticks_per_trial = 30

g = NEAT.Genome(0, 8 +1, 0, 4, False,
                NEAT.ActivationFunction.TANH, NEAT.ActivationFunction.TANH, 0, params, 0)
pop = NEAT.Population(g, params, True, 1.0, rnd.randint(0, 1000))


hof = []
maxf_ever = 0

env = gym.make('LunarLander-v2')

def interact_with_nn(net):
    global out
    inp = observation.tolist()
    net.Input(inp + [1.0])
    #print(inp)
    net.ActivateLeaky(0.1)
    out = list(net.Output())
    #print(np.argmax(list(out)))
    #out[0] *= 10.0
    #if out[0] < 0.0: out[0] = -2.0
    #if out[0] > 0.0: out[0] = 2.0
    return inp


def do_trial(net):
    global observation, reward, t, img, action, done, info, avg_reward
    observation = env.reset()
    net.Flush()

    f = 0
    for t in range(ticks_per_trial):

        if render_during_training:
            time.sleep(0.01)
            env.render()

        # interact with NN
        inp = interact_with_nn(net)

        if render_during_training:
            img = viz.Draw(net)
            cv2.imshow("current best", img)
            cv2.waitKey(1)

        action = np.argmax(out)
        observation, reward, done, info = env.step(action)
        if done: break

        f += reward

    return f

def evaluate(genome):
    net = NEAT.NeuralNetwork()
    genome.BuildPhenotype(net)

    avg_reward = 0

    for trial in range(trials):

        avg_reward += do_trial(net)

    avg_reward /= trials

    #print(avg_reward)

    genome.SetFitness(1000000 + avg_reward)
    return avg_reward

try:

    for generation in range(max_generations):
        genome_list = NEAT.GetGenomeList(pop)
        fitness_list = EvaluateGenomeList_Serial(genome_list, evaluate, display=False)
        # NEAT.ZipFitness(genome_list, fitness_list)
        maxf = max(fitness_list)

        # for i_episode, genome in enumerate(NEAT.GetGenomeList(pop)):
        #     evaluate(genome)            
        # maxf = max([x.GetFitness() for x in NEAT.GetGenomeList(pop)])


        print('Generation: {}/{}, max fitness: {}'.format(generation, max_generations - 1, maxf))

        if maxf > maxf_ever:
            hof.append(pickle.dumps(pop.GetBestGenome()))
            maxf_ever = maxf

        pop.Epoch()

except KeyboardInterrupt:
    pass


print('Replaying forever..')

if hof:
    while True:
        try:
            observation = env.reset()
            net = NEAT.NeuralNetwork()
            g = pickle.loads(hof[-1])
            g.BuildPhenotype(net)
            reward = 0

            for t in range(250):

                time.sleep(0.01)
                env.render()

                # interact with NN
                interact_with_nn(net)

                # render NN
                img = viz.Draw(net)
                cv2.imshow("current best", img)
                cv2.waitKey(1)

                action = np.argmax(out)
                observation, reward, done, info = env.step(action)

                if done:
                    break

        except Exception as ex:
            print(ex)
            time.sleep(0.2)

