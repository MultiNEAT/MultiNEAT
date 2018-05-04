from ._MultiNEAT import NeuralNetwork
from enum import IntFlag, auto


class Print(IntFlag):
    NONE = 0
    RUN = auto()
    GENERATION = auto()
    EVALUATION = auto()
    EVALUATION_TRIAL = auto()
    SOLUTION_FOUND = auto()
    SOLUTION_NOT_FOUND = auto()
    ALL = RUN | GENERATION | EVALUATION | EVALUATION_TRIAL | SOLUTION_FOUND | SOLUTION_NOT_FOUND


class DummyEvolutionReporter:
    def run(self, current, maximum):
        return DummyEvolutionContextManager()

    def generation(self, current, maximum):
        return DummyEvolutionContextManager()

    def evaluation(self, current, maximum, genome):
        return DummyEvolutionContextManager()

    def evaluation_trial(self, current, maximum, genome):
        return DummyEvolutionContextManager()

    def solution_found(self, population, generation):
        pass

    def solution_not_found(self, population):
        pass


class DummyEvolutionContextManager:
    def __enter__(self):
        pass

    def __exit__(self, type, value, traceback):
        pass


class CollectingEvolutionReporter(DummyEvolutionReporter):
    def __init__(self, *args, **kwargs):
        self.last_run = None
        self.max_run = None

        self.last_generation = None
        self.max_generation = None

        self.last_evaluated_genome = None

        self.last_evaluation_trial = None
        self.max_evaluation_trial = None
        self.last_evaluation_trial_genome = None

        self.solution_generation = None
        self.solution_population = None
        self.solution_not_found_population = None

    def run(self, current, maximum):
        self.last_run = current
        self.max_run = maximum

        return super().run(current, maximum)

    def generation(self, current, maximum):
        self.last_generation = current
        self.max_generation = maximum

        return super().generation(current, maximum)

    def evaluation(self, genome):
        self.last_evaluated_genome = genome

        return super().evaluation(genome)

    def evaluation_trial(self, current, maximum, genome):
        self.last_evaluation_trial = current
        self.max_evaluation_trial = maximum
        self.last_evaluation_trial_genome = genome

        return super().evaluation_trial(current, maximum, genome)

    def solution_found(self, population, generation):
        self.solution_generation = generation
        self.solution_population = population

    def solution_not_found(self, population):
        self.solution_not_found_population = population


class PrintingEvolutionReporter:
    def __init__(self, flags=Print.NONE):
        self.flags = flags

    def run(self, current, maximum):
        if Print.RUN in self.flags:
            return PrintingEvolutionContextManager("run", current, maximum)
        else:
            return DummyEvolutionContextManager()

    def generation(self, current, maximum):
        if Print.GENERATION in self.flags:
            return PrintingEvolutionContextManager("generation", current, maximum)
        else:
            return DummyEvolutionContextManager()

    def evaluation(self, genome):
        if Print.EVALUATION in self.flags:
            return PrintingEvolutionContextManager("evaluation")
        else:
            return DummyEvolutionContextManager()

    def evaluation_trial(self, current, maximum, genome):
        if Print.EVALUATION_TRIAL in self.flags:
            return PrintingEvolutionContextManager("evaluation trial", current, maximum)
        else:
            return DummyEvolutionContextManager()

    def solution_found(self, population, generation):
        if Print.SOLUTION_FOUND in self.flags:
            print("🎉🎉🎉 Solution found in {} generations".format(generation),
                  self.best_genome_stats(population))

    def solution_not_found(self, population):
        if Print.SOLUTION_NOT_FOUND in self.flags:
            print("😭😭😭 Solution not found")

    def best_genome_stats(self, population):
        net = NeuralNetwork()
        population.GetBestGenome().BuildPhenotype(net)

        return "complexity ({}, {})".format(net.NumHiddenNeurons(), net.NumConnections())


class PrintingEvolutionContextManager:
    def __init__(self, name, current=None, maximum=None):
        self.name = name
        self.current = current
        self.maximum = maximum

    def __enter__(self):
        print("started", self.name, self.current_maximum())

    def __exit__(self, type, value, traceback):
        print("ended", self.name, self.current_maximum())

    def current_maximum(self):
        if self.current and self.maximum:
            return "{}/{}".format(self.current, self.maximum)
        else:
            return ""
