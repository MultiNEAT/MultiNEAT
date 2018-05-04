"""
Interface for an experiment used by runner during evaluation stage
"""

class Experiment:
    def fitness(self, network):
        """Evalutes network and returns its fitness"""
        return 0

    def is_solved(self, best_fitness):
        """Reports if best_fitness is sufficient solution and evolution can be stopped"""
        return False
