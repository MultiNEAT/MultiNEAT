
class EvolutionReporter:
    def run_started(self): pass
    def run_ended(self, start_context): pass

    def generation_started(self): pass
    def generation_ended(self, start_context): pass

    def evaluation_started(self, genome): pass
    def evaluation_ended(self, start_context): pass

    def evaluation_trial_started(self, genome): pass
    def evaluation_trial_ended(self, start_context): pass

    def solution_found(self, population): pass
    def solution_not_found(self, population): pass
