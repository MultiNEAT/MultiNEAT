

class Runner:
  def __init__(self, *args, **kwargs):
    self.population = None
    self.generations_to_solve = None

  def create_seed_population(self):
    """Should return population"""
    pass

  def solve(self, population):

    pass

  def run(self):
    self.population = self.create_seed_population()
    self.solve(self.population)

