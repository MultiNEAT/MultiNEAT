| **`Linux and macOS`** | **`Windows`** |
|-----------------|---------------------|
| [![Build Status](https://travis-ci.org/MultiNEAT/MultiNEAT.svg?branch=master)](https://travis-ci.org/MultiNEAT/MultiNEAT) | ![Build Status](https://ci.appveyor.com/api/projects/status/yips7ifgdqfqvabt/branch/master?svg=true&passingText=master%20-%20OK) |

[![Conda Version](https://anaconda.org/anton.matosov/multineat/badges/version.svg)](https://anaconda.org/anton.matosov/multineat)

[![Conda Platforms](https://anaconda.org/anton.matosov/multineat/badges/platforms.svg)](https://anaconda.org/anton.matosov/multineat)

[![Install with Conda](https://anaconda.org/anton.matosov/multineat/badges/installer/conda.svg)](https://anaconda.org/anton.matosov/multineat)

# About MultiNEAT

MultiNEAT is a portable software library for performing neuroevolution, a form of machine learning that
trains neural networks with a genetic algorithm. It is based on NEAT, an advanced method for evolving
neural networks through complexification. The neural networks in NEAT begin evolution with very simple
genomes which grow over successive generations. The individuals in the evolving population are grouped
by similarity into species, and each of them can compete only with the individuals in the same species.

The combined effect of speciation, starting from the simplest initial structure and the correct
matching of the genomes through marking genes with historical markings yields an algorithm which
is proven to be very effective in many domains and benchmarks against other methods.

NEAT was developed around 2002 by Kenneth Stanley in the University of Texas at Austin.

### License

GNU Lesser General Public License v3.0 

### Documentation
[http://multineat.com/docs.html](http://multineat.com/docs.html)

#### To install

Right now prebuild mulitneat is available from anton.matosov personal Conda channel and dependencies from conda-forge channel:

  ```bash
  conda install multineat -c anton.matosov -c conda-forge
  ```

#### Supported configurations:

|                | **`Python 2.7`** | **`Python 3.5`** | **`Python 3.6`** |
|---------------:|:----------------:|:----------------:|:----------------:|
| **`Linux`**    |       👍         |       👍          |        👍        |
| **`macOS`**    |       👍         |       👍          |        👍        |
| **`Windows`**  |       👎         |       👍          |        👍        |

Building MultiNEAT on Windows with python 2.7 is not possible, becuase it uses compiler from VS2008 that doesn't support C++11 features required by the library.

##### P.S.

Conda-forge recipe is in review and will be merged soon. This will allow to install from conda-forge channel directly, without using Anton's personal one.

#### To compile

From now on only boost-python bindings are supported. So make sure to install boost and boost-python (e.g. from conda-forge) and as usual:

  ```bash
  python setup.py build_ext
  python setup.py install
  ```
