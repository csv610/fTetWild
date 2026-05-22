# fTetWild (Refactored Port)

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)
![](./figs/1k.jpg)

## Disclaimer & Credit
This repository is a **port and refactor** of the original [fTetWild](https://github.com/Yixin-Hu/fTetWild) project. 

**Absolutely no credit is taken from the original authors** for the underlying algorithms, research, or foundational implementation. This work belongs entirely to:

> **Yixin Hu, Teseo Schneider, Bolun Wang, Denis Zorin, Daniele Panozzo.**  
> *Fast Tetrahedral Meshing in the Wild*  
> ACM Transactions on Graphics (SIGGRAPH 2020)

The primary goal of this fork is to modernize the codebase, improve maintainability, and provide better integration tools for the modern C++ ecosystem.

```bibtex
@article{10.1145/3386569.3392385,
author = {Hu, Yixin and Schneider, Teseo and Wang, Bolun and Zorin, Denis and Panozzo, Daniele},
title = {Fast Tetrahedral Meshing in the Wild},
year = {2020},
issue_date = {July 2020},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
volume = {39},
number = {4},
issn = {0730-0301},
url = {https://doi.org/10.1145/3386569.3392385},
doi = {10.1145/3386569.3392385},
journal = {ACM Trans. Graph.},
month = jul,
articleno = {117},
numpages = {18},
keywords = {mesh generation, robust geometry processing, tetrahedral meshing}
}
```

## Refactored Contributions

This version introduces the following enhancements and refactorings:

*   🚀 **C++20 Modernization**: The entire codebase has been upgraded to the **C++20** standard, leveraging modern language features for better performance and safety.
*   📦 **Improved Build System**: Refactored CMake configuration to use modern patterns, including `FetchContent` for automatic dependency management and improved support for system-installed libraries (like Eigen and GMP).
*   🧪 **Robust Testing Suite**: Added a comprehensive testing framework using Catch2, including both unit tests for core components and functional tests for the end-to-end pipeline.
*   📊 **Benchmarking & Analysis**: Integrated new performance benchmarking tools (`ftetwild_bench`) and Python scripts for scalability and complexity analysis.
*   ⚙️ **CI/CD Integration**: Added GitHub Actions workflows for continuous integration and automated testing across different environments.
*   📖 **Extended Documentation**: Detailed technical guides and usage instructions have been consolidated into the [docs/](./docs/) directory.

## Installation via CMake

- Compile the code using cmake (requires C++20):

```bash
cd fTetWild
mkdir build
cd build
cmake ..
make -j8
```

### Dependencies
The project uses `FetchContent` to manage most dependencies automatically. However, you may need to install `gmp` and `eigen` manually if they are not found:

- **macOS**: `brew install gmp eigen`
- **Linux**: `sudo apt-get install libgmp-dev libeigen3-dev`

## Testing & Benchmarking

### Running Tests
To run unit and functional tests:
```bash
ctest --output-on-failure
```

### Performance Benchmarks
To run the performance benchmark on a specific mesh:
```bash
./bench/ftetwild_bench ../tests/bunny.off
```

### Scalability Analysis
To analyze thread scalability:
```bash
python3 ../bench/scalability_analysis.py ../tests/bunny.off
```

## Usage

### Command Line Switches
```bash
./FloatTetwild_bin [OPTIONS]
Options:
  -h,--help                   Print this help message and exit
  -i,--input TEXT:FILE        Input surface mesh (.off/.obj/.stl/.ply)
  -o,--output TEXT            Output tetmesh (.msh)
  -l,--lr FLOAT               ideal_edge_length = diag_of_bbox * L (default: 0.05)
  -e,--epsr FLOAT             epsilon = diag_of_bbox * EPS (default: 1e-3)
  --max-threads UINT          Maximum number of threads
```

For detailed information on all features, refer to the [User Guide](./docs/UserGuide.md).
