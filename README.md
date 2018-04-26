
Current build status
====================
| |  |
| --- | --- |
| Master branch | [![Linux and mac Build Status](https://img.shields.io/travis/MultiNEAT/MultiNEAT.svg?longCache=true&style=flat&label=Linux/macOS)](https://travis-ci.org/MultiNEAT/MultiNEAT) [![Windows Build Status](https://img.shields.io/appveyor/ci/anton-matosov/multineat-w3958/master.svg?longCache=true&style=flat&label=Windows)](https://ci.appveyor.com/project/anton-matosov/multineat-w3958) |
| Conda-forge | [![Linux](https://img.shields.io/circleci/project/github/conda-forge/multineat-feedstock/master.svg?label=Linux)](https://circleci.com/gh/conda-forge/multineat-feedstock) [![OSX](https://img.shields.io/travis/conda-forge/multineat-feedstock/master.svg?label=macOS)](https://travis-ci.org/conda-forge/multineat-feedstock) [![Windows](https://img.shields.io/appveyor/ci/conda-forge/multineat-feedstock/master.svg?label=Windows)](https://ci.appveyor.com/project/conda-forge/multineat-feedstock/branch/master) |

Current release info
====================

| Name | Downloads | Version | Platforms |
| --- | --- | --- | --- |
| [![Conda Recipe](https://img.shields.io/badge/recipe-multineat-green.svg)](https://anaconda.org/conda-forge/multineat) | [![Conda Downloads](https://img.shields.io/conda/dn/conda-forge/multineat.svg)](https://anaconda.org/conda-forge/multineat) | [![Conda Version](https://img.shields.io/conda/vn/conda-forge/multineat.svg)](https://anaconda.org/conda-forge/multineat) | [![Conda Platforms](https://img.shields.io/conda/pn/conda-forge/multineat.svg)](https://anaconda.org/conda-forge/multineat) |


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

Prebuilt MultiNEAT package is available from conda-forge:

  ```bash
  conda install multineat -c conda-forge
  ```

Conda-forge feedstock recipe can be found [here](https://github.com/conda-forge/multineat-feedstock).

#### Supported configurations:

|                | **`Python 2.7`** | **`Python 3.5`** | **`Python 3.6`** |
|---------------:|:----------------:|:----------------:|:----------------:|
| **`Linux`**    |       👍         |       👍          |        👍        |
| **`macOS`**    |       👍         |       👍          |        👍        |
| **`Windows`**  |       👎         |       👍          |        👍        |

Building MultiNEAT on Windows with python 2.7 is not possible, becuase it uses compiler from VS2008 that doesn't support C++11 features required by the library.

#### To compile

From now on only boost-python bindings are supported. So make sure to install boost and boost-python (e.g. from conda-forge) and as usual:

  ```bash
  python setup.py build_ext
  python setup.py install
  ```
