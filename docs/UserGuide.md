# fTetWild User Guide

## Introduction
fTetWild (Floating-Point Tetrahedral Meshing) is a robust tetrahedral meshing algorithm designed to convert complex, potentially "dirty" surface meshes into high-quality tetrahedral meshes. Unlike traditional meshers that struggle with self-intersections, gaps, or non-manifold geometry, fTetWild is built to be "always successful" by utilizing a floating-point-based approach combined with rational predicates where necessary.

## Core Idea: Floating-Point Tetrahedral Meshing
The core philosophy of fTetWild is to provide a robust bridge between imperfect surface geometry and volumetric simulation.

1.  **Enveloping**: The algorithm ensures that the output mesh stays within a tight, user-defined envelope of the input surface.
2.  **Incremental Insertion**: It starts with a bounding box and incrementally inserts input vertices and edges.
3.  **Local Refinement and Optimization**: It uses operations like edge splitting, collapsing, and swapping to improve mesh quality while respecting the geometric constraints.
4.  **Robust Predicates**: By using exact predicates (via libigl and GMP), fTetWild avoids the common pitfalls of floating-point inaccuracies during critical geometric checks, ensuring no inverted elements are produced.

## Improvements from Previous Versions
This version of fTetWild introduces several significant engineering and algorithmic improvements:

*   **Modern C++20 Support**: The codebase has been upgraded to C++20, leveraging modern language features for better performance and maintainability.
*   **System Eigen Integration**: The build system now supports using a system-wide Eigen installation, reducing build times and improving integration with existing development environments.
*   **Enhanced Dependency Management**: Utilizing CMake\s `FetchContent`, the project autonomously manages its dependencies (libigl, spdlog, fmt, geogram, TBB), ensuring a seamless "out-of-the-box" experience.
*   **Improved Logging**: Integration with `spdlog` provides structured, high-performance logging with configurable verbosity levels.

## Comparison with TetGen
| Feature | TetGen | fTetWild |
| :--- | :--- | :--- |
| **Input Sensitivity** | Highly sensitive; requires clean, manifold water-tight surfaces. | Extremely robust; handles self-intersections, gaps, and open boundaries. |
| **Geometric Accuracy** | High (for clean inputs). | Controlled via a user-defined envelope (`eps`). |
| **Failure Rate** | Can fail on "dirty" real-world geometry. | Designed to be "always successful." |
| **Speed** | Generally faster for simple/clean geometry. | Slower due to the robust processing and optimization passes. |
| **Output Quality** | Excellent for conforming Delaunay. | Optimized for simulation-ready elements with controlled aspect ratios. |

## New Features
*   **Exact Envelope (`NEW_ENVELOPE`)**: An optional feature that uses a more precise envelope calculation to ensure the output mesh follows the input geometry even more closely in narrow regions.
*   **CSG (Constructive Solid Geometry)**: Support for Boolean operations (Union, Intersection, Difference) via a JSON-defined CSG tree or command-line tags.
*   **Manifold Surface Forcing**: A feature to force the output boundary to be manifold, even if the input was not.
*   **Coarsening Optimization**: A dedicated pass (`--coarsen`) to reduce the element count in interior regions while maintaining quality and accuracy.
*   **Sizing Fields**: Support for background meshes to define spatially varying element sizes.

## Testing and Benchmarking

### Unit and Functional Testing
fTetWild includes a comprehensive test suite managed via `ctest`. It includes unit tests for geometric predicates and functional tests that verify the full meshing pipeline.

```bash
ctest --output-on-failure
```

### Performance Benchmarking
A dedicated benchmark utility is available to measure the execution time of the core pipeline:

```bash
./bench/ftetwild_bench <input_mesh.off> [num_threads]
```

### Scalability Analysis
The algorithm utilizes TBB for multi-core parallelism. You can evaluate how the performance scales on your hardware using the provided script:

```bash
python3 ../bench/scalability_analysis.py ../tests/bunny.off
```

**Example Results (Bunny Mesh):**
- Sequential: ~7.5s
- 8 Threads: ~6.0s
- Peak Efficiency is usually reached on larger meshes where the work-per-thread is higher.

### Complexity Scaling
fTetWild demonstrates near-linear $O(N)$ scaling with respect to the number of generated tetrahedra. To analyze complexity scaling on your system:

```bash
python3 ../bench/complexity_analysis.py
```

**Observed Performance:**
- Average cost: ~30-40 microseconds per generated tetrahedron.
- Scaling remains stable from thousands to millions of elements.

## Getting Started
To generate a tetrahedral mesh from a surface mesh (`input.off`):

```bash
./FloatTetwild_bin -i input.off -o output.msh
```

### Common Options:
*   `-l / --lr`: Set the ideal edge length relative to the bounding box diagonal (default: 0.05).
*   `-e / --epsr`: Set the envelope tolerance relative to the bounding box diagonal (default: 1e-3).
*   `--is-quiet`: Mute the console output.
*   `--max-threads`: Specify the number of TBB threads to use.
