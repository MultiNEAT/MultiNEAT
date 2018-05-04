
from ._MultiNEAT import *
from .viz import *
from .runner import *
from .gym_runner import *
from .experiment import *
from .evolution_reporter import *


try:
    import networkx as nx

    def Genome2NX(g):

        nts = g.GetNeuronTraits()
        lts = g.GetLinkTraits()
        gr = nx.DiGraph()

        for i, tp, traits in nts:
            gr.add_node(i, **traits)

        for inp, outp, traits in lts:
            gr.add_edge(inp, outp, **traits)

        gr.genome_traits = g.GetGenomeTraits()

        return gr
except:
    pass
