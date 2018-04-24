#!/usr/bin/python

from __future__ import print_function
from setuptools import setup, Extension
import sys
import os
import psutil

# monkey-patch for parallel compilation
def parallelCCompile(self, sources, output_dir=None, macros=None, include_dirs=None, debug=0, extra_preargs=None, extra_postargs=None, depends=None):
    # those lines are copied from distutils.ccompiler.CCompiler directly
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(output_dir, macros, include_dirs, sources, depends, extra_postargs)
    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)
    # parallel code
    
    N = psutil.cpu_count(logical=False) # number of parallel compilations
    import multiprocessing.pool
    def _single_compile(obj):
        try: src, ext = build[obj]
        except KeyError: return
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)
    # convert to list, imap is evaluated on-demand
    list(multiprocessing.pool.ThreadPool(N).imap(_single_compile,objects))
    return objects

import distutils.ccompiler
distutils.ccompiler.CCompiler.compile=parallelCCompile


''' Note:

to build Boost.Python on Windows with mingw

bjam target-os=windows/python=3.4 toolset=gcc variant=debug,release link=static,shared threading=multi runtime-link=shared cxxflags="-include cmath "


also insert this on top of boost/python.hpp :

#include <cmath>   //fix  cmath:1096:11: error: '::hypot' has not been declared

'''


def getExtensions():
    platform = sys.platform

    extensionsList = []
    sources = ['src/Genome.cpp',
               'src/Innovation.cpp',
               'src/NeuralNetwork.cpp',
               'src/Parameters.cpp',
               'src/PhenotypeBehavior.cpp',
               'src/Population.cpp',
               'src/Random.cpp',
               'src/Species.cpp',
               'src/Substrate.cpp',
               'src/Utils.cpp']

    extra = ['-march=native',
             '-g'
             ]

    if platform == 'darwin':
        extra += ['-stdlib=libc++',
             '-std=c++11',]
    else:
        extra += ['-std=gnu++11']

    is_windows = 'win' in platform and platform != 'darwin'
    if is_windows:
        extra.append('/EHsc')
    else:
        extra.append('-w')

    prefix = os.getenv('PREFIX')
    if prefix and len(prefix) > 0:
        extra += ["-I{}/include".format(prefix)]

    is_python_2 = sys.version_info[0] < 3
    python_version_string = "{}{}".format(sys.version_info[0], sys.version_info[1])

    sources.insert(0, 'src/PythonBindings.cpp')

    if is_windows:
        if is_python_2:
            raise RuntimeError("Python prior to version 3 is not supported on Windows due to limits of VC++ compiler version")

    libs = ['boost_system', 'boost_serialization',
            'boost_python' + python_version_string, "boost_numpy" + python_version_string]

    extra.extend(['-DUSE_BOOST_PYTHON'])
    extensionsList.append(Extension('MultiNEAT._MultiNEAT',
                                    sources,
                                    libraries=libs,
                                    extra_compile_args=extra))

    return extensionsList


setup(name='multineat',
      version='0.5.3', # Update version in conda/meta.yaml as well
      packages=['MultiNEAT'],
      ext_modules=getExtensions())
