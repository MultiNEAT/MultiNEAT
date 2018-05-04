
class XorExperiment:
    def __init__(self, depth = 5, *args, **kwargs):
        self.depth = depth

    def fitness(self, net):
        error = 0

        error += self._evaluate(net, input = [1, 0, 1], output = 1)
        error += self._evaluate(net, input = [0, 1, 1], output = 1)
        error += self._evaluate(net, input = [1, 1, 1], output = 0)
        error += self._evaluate(net, input = [0, 0, 1], output = 0)

        return (4 - error) ** 2

    def _evaluate(self, net, input, output):
        net.Flush()
        net.Input(input)
        [net.Activate() for _ in range(self.depth)]
        
        return abs(net.Output()[0] - output)
