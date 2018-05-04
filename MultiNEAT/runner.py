import time

from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor, as_completed

class Experiment:
    def fitness(self, network):
        """Evalutes network and returns its fitness"""
        return 0

    def is_solved(self, best_fitness):
        """Reports if best_fitness is sufficient solution and evolution can be stopped"""
        return False


class RunnerDelegate:
    def create_seed_population(self):
        """Should return population"""
        pass

    def build_phenotype(self, genome):
        return None


class Runner:
    def __init__(self, delegate, experiment, *args, **kwargs):
        self.delegate = delegate
        self.experiment = experiment

        self.max_generations = 1
        self.population = None
        self.generations_to_solve = 0
        self.total_time = 0
        self.time_per_generation = 0

    def __evolve(self, population):
        """Main method that runs evolution"""
        self.generations_to_solve = 0

        self.population = population
        for generation in range(self.max_generations):
            fitness_list = EvaluateSerial(
                self.population, self.__evaluate_fitness, display=False)

            best_fitness = max(fitness_list)
            if self.experiment.is_solved(best_fitness):
                self.generations_to_solve = generation
                break

            self.population.Epoch()  # evolution happens here

    def __evaluate_fitness(self, genome):
        """Evaluation fitness single genome"""
        network = self.delegate.build_phenotype(genome)
        return self.experiment.fitness(network)

    def run(self, max_generations):
        """Run evolution for maximum of max_generations"""
        self.max_generations = max_generations

        start_time = time.time()

        population = self.delegate.create_seed_population()
        self.__evolve(population)

        self.total_time = time.time() - start_time
        self.time_per_generation = (
            self.total_time / self.generations_to_solve) * 1000


# Get all genomes from the population
def GetGenomeList(pop):
    genome_list = []
    for s in pop.Species:
        for i in s.Individuals:
            genome_list.append(i)
    return genome_list


# Just set the fitness values to the genomes
def ZipFitness(genome_list, fitness_list):
    for genome, fitness in zip(genome_list, fitness_list):
        genome.SetFitness(fitness)
        genome.SetEvaluated()

try:
    from IPython.display import clear_output
    from ipyparallel import Client

    ipython_installed = True
except:
    ipython_installed = False

try:
    from progressbar import ProgressBar
    pbar_installed = True
except:
    pbar_installed = False


def static_vars(**kwargs):
    """ A little helper that allows to add static vars to functions """
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate


def EvaluateSerial(population, evaluator, display=False, show_elapsed=False):
    genome_list = GetGenomeList(population)
    fitness_list = EvaluateGenomeList_Serial(
        genome_list, evaluator, display=display, show_elapsed=show_elapsed)
    ZipFitness(genome_list, fitness_list)

    return fitness_list


def EvaluateParallel(population, evaluator,
                     cores=None, display=False, ipython_client=None):
    genome_list = GetGenomeList(population)
    fitness_list = EvaluateGenomeList_Parallel(
        genome_list, evaluator, cores=cores, display=display, ipython_client=ipython_client)
    ZipFitness(genome_list, fitness_list)

    return fitness_list


# Evaluates all genomes in sequential manner (using only 1 process) and
# returns a list of corresponding fitness values.
# evaluator is a callable that is supposed to take Genome as argument and
# return a double
def EvaluateGenomeList_Serial(genome_list, evaluator, display=False, show_elapsed=False):
    fitnesses = []
    count = 0

    if display and show_elapsed:
        curtime = time.time()

    if display and pbar_installed:
        pbar = ProgressBar()
        pbar.max_value = len(genome_list)
        pbar.min_value = 0

    for i, g in enumerate(genome_list):
        f = evaluator(g)
        fitnesses.append(f)

        if display:
            if not pbar_installed:
                if ipython_installed:
                    clear_output(wait=True)
                print('Individuals: (%s/%s) Fitness: %3.4f' %
                      (count, len(genome_list), f))
            else:
                pbar.update(i)
        count += 1

    if display and pbar_installed:
        pbar.finish()

    if display and show_elapsed:
        elapsed = time.time() - curtime
        print('seconds elapsed: %s' % elapsed)

    return fitnesses


# Evaluates all genomes in parallel manner (many processes) and returns a
# list of corresponding fitness values.
# evaluator is a callable that is supposed to take Genome as argument and return a double
@static_vars(executor=None)
def EvaluateGenomeList_Parallel(genome_list, evaluator,
                                cores=None, display=False, ipython_client=None):
    ''' If ipython_client is None, will use concurrent.futures. 
    Pass an instance of Client() in order to use an IPython cluster '''
    fitnesses = []
    curtime = time.time()

    if not cores:
        try:
            import psutil
            cores = psutil.cpu_count(logical=False)
        except:
            cores = 2
    batch_size = min(100, int(len(genome_list) / cores))
    if ipython_client is None or not ipython_installed:

        executor = EvaluateGenomeList_Parallel.executor
        if not executor:
            executor = ProcessPoolExecutor(max_workers=cores)
            EvaluateGenomeList_Parallel.executor = executor

        for i, fitness in enumerate(executor.map(evaluator, genome_list, chunksize=batch_size)):
            fitnesses += [fitness]

            if display:
                if ipython_installed:
                    clear_output(wait=True)
                print('Individuals: (%s/%s) Fitness: %3.4f' %
                      (i, len(genome_list), fitness))
    else:
        if type(ipython_client) == Client:
            lbview = ipython_client.load_balanced_view()
            amr = lbview.map(evaluator, genome_list, ordered=True, block=False)
            for i, fitness in enumerate(amr):
                if display:
                    if ipython_installed:
                        clear_output(wait=True)
                    print('Individual:', i, 'Fitness:', fitness)
                fitnesses.append(fitness)
        else:
            raise ValueError(
                'Please provide valid IPython.parallel Client() as ipython_client')

    elapsed = time.time() - curtime

    if display:
        print('seconds elapsed: %3.4f' % elapsed)

    return fitnesses
