from ._MultiNEAT import NeuralNetwork


class EvolutionReporter:
    def run(self, current, maximum): return DummyEvolutionContextManager()

    def generation(
        self, current, maximum): return DummyEvolutionContextManager()

    def evaluation(self, genome): return DummyEvolutionContextManager()

    def evaluation_trial(self, genome): return DummyEvolutionContextManager()

    def solution_found(self, population, generation): pass

    def solution_not_found(self, population): pass


class DummyEvolutionContextManager:
    def __enter__(self):
        pass

    def __exit__(self, type, value, traceback):
        pass


class PrintingEvolutionReporter:
    def run(self, current, maximum):
        return PrintingEvolutionContextManager("run", current, maximum)

    def generation(self, current, maximum):
        return PrintingEvolutionContextManager("generation", current, maximum)

    def evaluation(self, genome):
        return PrintingEvolutionContextManager("evaluation")

    def evaluation_trial(self, genome):
        return PrintingEvolutionContextManager("evaluation trial")

    def solution_found(self, population, generation):
        print("🎉🎉🎉 Solution found in {} generations".format(generation),
              self.best_genome_stats(population))

    def solution_not_found(self, population):
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
