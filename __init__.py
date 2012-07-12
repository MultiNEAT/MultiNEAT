import sys
import multiprocessing as mpc
import time
from progressbar import ProgressBar, Counter, ETA, AnimatedMarker
from Release.libNEAT import *
import cv2
import numpy as np

# Get all genomes from the population
def GetGenomeList(pop):
    genome_list = []
    for s in pop.Species:
        for i in s.Individuals:
            genome_list.append(i)
    return genome_list

RetrieveGenomeList = GetGenomeList
FetchGenomeList = GetGenomeList

# Evaluates all genomes in sequential manner (using only 1 process) and returns a list of corresponding fitness values and the time it took
# evaluator is a callable that is supposed to take Genome as argument and return a double
def EvaluateGenomeList_Serial(genome_list, evaluator):
    fitnesses = []
    curtime = time.time()
    widg = ['Individuals: ', Counter(), ' of ' + str(len(genome_list)), ' ', ETA(), ' ', AnimatedMarker()]
    progress = ProgressBar(maxval=len(genome_list), widgets=widg).start()
    count = 0
    for g in genome_list:
        f = evaluator(g)
        fitnesses.append(f)
        progress.update(count)
        count += 1
    progress.finish()
    elapsed = time.time() - curtime
    print 'seconds elapsed:', elapsed
    return (fitnesses, elapsed)
    
# Evaluates all genomes in parallel manner (many processes) and returns a list of corresponding fitness values and the time it took
# evaluator is a callable that is supposed to take Genome as argument and return a double
def EvaluateGenomeList_Parallel(genome_list, evaluator, cores):
    fitnesses = []
    pool = mpc.Pool(processes=cores)
    curtime = time.time()
    widg = ['Individuals: ', Counter(), ' of ' + str(len(genome_list)), ' ', ETA(), ' ', AnimatedMarker()]
    progress = ProgressBar(maxval=len(genome_list), widgets=widg).start()
    for i, fitness in enumerate(pool.imap(evaluator, genome_list)):
        progress.update(i)
        fitnesses.append(fitness)
    progress.finish()
    elapsed = time.time() - curtime
    print 'seconds elapsed:', elapsed
    pool.close()
    pool.join()
    return (fitnesses, elapsed)
    
# Just set the fitness values to the genomes
def ZipFitness(genome_list, fitness_list):
    for g,f in zip(genome_list, fitness_list):
        g.SetFitness(f)
        
def Scale(a, a_min, a_max, a_tr_min, a_tr_max):
    t_a_r = a_max - a_min;
    if t_a_r == 0:
        return a_max
    
    t_r = a_tr_max - a_tr_min;
    rel_a = (a - a_min) / t_a_r;
    return a_tr_min + t_r * rel_a;

def Clamp(a, min, max):
    if a < min:
        return min
    elif a > max:
        return max
    else:
        return a
    
def AlmostEqual(a,b, margin):
    if abs(a-b) > margin:
        return False
    else:
        return True

# Neural Network display code
# rect is a tuple in the form (x, y, size_x, size_y)
MAX_DEPTH = 250
def DrawPhenotype(image, rect, nn, neuron_radius=10, max_line_thickness=3):
    for i, n in enumerate(nn.neurons):
        nn.neurons[i].x = 0
        nn.neurons[i].y = 0
    
    rect_x = rect[0]
    rect_y = rect[1]
    rect_x_size = rect[2]
    rect_y_size = rect[3]
    
    depth = 0
    depth_inc = 1.0 / MAX_DEPTH
    # for every depth, count how many nodes are on this depth
    all_depths = np.linspace(0.0, 1.0, MAX_DEPTH)#np.concatenate( np.arange(0.0, 1.0, depth_inc, dtype=np.float32), [1.0] )
    for depth in all_depths:
        neuron_count = 0
        for neuron in nn.neurons:
            if AlmostEqual(neuron.split_y, depth, 1.0 / (MAX_DEPTH+1)):
                neuron_count += 1
        if neuron_count == 0:
            continue
        
        # calculate x positions of neurons
        j = 0
        xxpos = rect_x_size / (1 + neuron_count)
        for neuron in nn.neurons:
            if AlmostEqual(neuron.split_y, depth, 1.0 / (MAX_DEPTH+1)):
                neuron.x = rect_x + xxpos + j * (rect_x_size) / (2 + neuron_count)
                j += 1
    
    # calculate y positions of nodes
    for neuron in nn.neurons:
        if neuron.split_y == 0.0:
            neuron.y = rect_y + neuron.split_y * (rect_y_size - neuron_radius) + neuron_radius
        elif neuron.split_y == 1.0:
            neuron.y = rect_y + neuron.split_y * (rect_y_size - neuron_radius)
        else:
            neuron.y = rect_y + neuron.split_y * (rect_y_size - neuron_radius) 

        
    # the positions of neurons is computed, now we draw
    
    # connections first
    max_weight = max([abs(x.weight) for x in nn.connections])
    for conn in nn.connections:
        thickness = conn.weight
        thickness = Scale(thickness, 0, max_weight, 1, max_line_thickness)
        thickness = Clamp(thickness, 1, max_line_thickness)
        
        w = Scale(abs(conn.weight), 0.0, max_weight, 0.0, 1.0)
        w = Clamp(w, 0.75, 1.0)
        
        if conn.recur_flag:
            if conn.weight < 0:
                # green weight
                color = (0, int(255.0 * w), 0 )
            else:
                # white weight
                color = (int(255.0 * w), int(255.0 * w), int(255.0 * w) )
        else:
            if conn.weight < 0:
                # blue weight
                color = (int(255.0 * w), 0, 0 )
            else:
                # red weight
                color = (0, 0, int(255.0 * w) )

        # if the link is looping back on the same neuron, draw it with ellipse
        if conn.source_neuron_idx == conn.target_neuron_idx:
            pass # todo: later
        else:
            # Draw a line
            pt1 = (int(nn.neurons[ conn.source_neuron_idx ].x), int(nn.neurons[ conn.source_neuron_idx ].y))
            pt2 = (int(nn.neurons[ conn.target_neuron_idx ].x), int(nn.neurons[ conn.target_neuron_idx ].y))
            cv2.line(image, pt1, pt2, color, int(thickness))
     
     # draw all neurons
    for neuron in nn.neurons:
        pt = (int( neuron.x ), int( neuron.y))
        cv2.circle(image, pt, neuron_radius, (255,255,255), -1) 


    