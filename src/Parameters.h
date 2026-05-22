// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#pragma once

#include <floattetwild/Types.hpp>

#include <array>
#include <vector>
#include <limits>
#include <cmath>

#include <geogram/mesh/mesh.h>

namespace floatTetWild {
namespace constants {
    // Box scaling - fraction of bounding box diagonal for padding
    constexpr Scalar kBoxScale = 1.0 / 15.0;

    // Relative tolerance for envelope (fraction of bounding box diagonal)
    constexpr Scalar kEpsilonRelative = 1e-3;

    // Ideal edge length as fraction of bounding box diagonal
    constexpr Scalar kIdealEdgeLengthRelative = 1.0 / 20.0;

    // Energy threshold for mesh quality
    constexpr Scalar kStopEnergy = 10.0;

    // Maximum optimization iterations
    constexpr int kMaxIterations = 80;

    // Envelope delta factor for multi-stage processing
    constexpr Scalar kEnvelopeDeltaFactor = 0.1;
    constexpr Scalar kEnvelopeCoplanarFactor = 0.2;
    constexpr Scalar kEnvelopeSimplificationFactor = 0.8;

    // Edge length thresholds for splitting/collapsing
    constexpr Scalar kSplitThresholdFactor = 4.0 / 3.0;
    constexpr Scalar kCollapseThresholdFactor = 4.0 / 5.0;

    // Stage limits
    constexpr int kMaxStage = 5;
    constexpr int kDefaultStage = 2;

    // Debug output thresholds
    constexpr Scalar kMinCoplanarEpsilon = 1e-6;
    constexpr Scalar kDampingFactor = 1.5;

    // Energy convergence thresholds
    constexpr Scalar kEnergyConvergenceThreshold = 5e-1;
    constexpr Scalar kAvgEnergyConvergenceThreshold = 0.1;

    // Mesh sizing parameters
    constexpr Scalar kRadiusScaleFactor = 1.8;

    // Filtering weight threshold
    constexpr Scalar kWeightThreshold = 0.5;

    // Energy and quality thresholds
    constexpr Scalar kFilterEnergyThreshold = 100.0;
    constexpr Scalar kQualityThreshold = 1e10;
    constexpr int kEnvelopeLogThreshold = 100000;
    constexpr Scalar kSurfaceNormalDotThreshold = 1.0 - 1e-6;
}

class Parameters {
public:
    std::string log_path = "";
    std::string input_path = "";
    std::string output_path = "";
    std::string tag_path = "";
    std::string postfix = "";

    std::string envelope_log = "";
    std::string envelope_log_csv = "";

    bool not_sort_input = false;
    bool correct_surface_orientation = false;
    bool is_quiet = false;
    int log_level = 3;

    bool smooth_open_boundary = false;
    bool manifold_surface = false;
    bool disable_filtering = false;
    bool use_floodfill = false;
    bool use_general_wn = false;
    bool use_input_for_wn = false;
    bool coarsen = false;

    bool apply_sizing_field = false;
    Eigen::VectorXd V_sizing_field;
    Eigen::VectorXi T_sizing_field;
    Eigen::VectorXd values_sizing_field;
    std::function<double(const Vector3&)> get_sizing_field_value;

#ifdef NEW_ENVELOPE
    std::vector<double> input_epsr_tags;
#endif

    // Box scale as fraction of bounding box diagonal
    Scalar box_scale = constants::kBoxScale;

    // Relative epsilon for envelope (fraction of diagonal)
    Scalar eps_rel = constants::kEpsilonRelative;

    // Ideal edge length as fraction of bounding box diagonal
    Scalar ideal_edge_length_rel = constants::kIdealEdgeLengthRelative;
    Scalar min_edge_len_rel = -1;

    // Absolute edge length (overrides relative if > 0)
    Scalar ideal_edge_length_abs = 0.0;

    int max_its = constants::kMaxIterations;
    Scalar stop_energy = constants::kStopEnergy;

#ifdef NEW_ENVELOPE
    int stage = 1;
#else
    int stage = constants::kDefaultStage;
#endif

    unsigned int num_threads = std::numeric_limits<unsigned int>::max();

    int stop_p = -1;

    Vector3 bbox_min;
    Vector3 bbox_max;
    Scalar bbox_diag_length = 0;
    Scalar ideal_edge_length = 0;
    Scalar ideal_edge_length_2 = 0;
    Scalar eps_input = 0;
    Scalar eps = 0;
    Scalar eps_delta = 0;
    Scalar eps_2 = 0;
    Scalar dd = 0;
    Scalar min_edge_length = 0;

    Scalar split_threshold = 0;
    Scalar collapse_threshold = 0;
    Scalar split_threshold_2 = 0;
    Scalar collapse_threshold_2 = 0;

    Scalar eps_coplanar = 0;
    Scalar eps_2_coplanar = 0;
    Scalar eps_simplification = 0;
    Scalar eps_2_simplification = 0;
    Scalar dd_simplification = 0;

    bool init(Scalar bbox_diag_l) {
        if (stage > constants::kMaxStage)
            stage = constants::kMaxStage;

        bbox_diag_length = bbox_diag_l;

        if (ideal_edge_length_abs > 0.0) {
            ideal_edge_length = ideal_edge_length_abs;
            ideal_edge_length_rel = ideal_edge_length / bbox_diag_length;
        }
        else {
            ideal_edge_length = bbox_diag_length * ideal_edge_length_rel;
        }
        ideal_edge_length_2 = ideal_edge_length * ideal_edge_length;

        eps_input = bbox_diag_length * eps_rel;
        dd = eps_input / constants::kDampingFactor;

#ifdef NEW_ENVELOPE
        double eps_usable = eps_input;
        eps_delta = eps_usable * constants::kEnvelopeDeltaFactor;
        eps = eps_usable - eps_delta * (stage - 1);
#else
        double eps_usable = eps_input - dd / std::sqrt(3.0);
        eps_delta = eps_usable * constants::kEnvelopeDeltaFactor;
        eps = eps_usable - eps_delta * (stage - 1);
#endif

        eps_2 = eps * eps;

        eps_coplanar = eps * constants::kEnvelopeCoplanarFactor;
        if (eps_coplanar > bbox_diag_length * constants::kMinCoplanarEpsilon)
            eps_coplanar = bbox_diag_length * constants::kMinCoplanarEpsilon;
        eps_2_coplanar = eps_coplanar * eps_coplanar;

        eps_simplification = eps * constants::kEnvelopeSimplificationFactor;
        eps_2_simplification = eps_simplification * eps_simplification;
        dd_simplification = dd / eps * eps_simplification;

        if (min_edge_len_rel < 0)
            min_edge_len_rel = eps_rel;
        min_edge_length = bbox_diag_length * min_edge_len_rel;

        split_threshold = ideal_edge_length * constants::kSplitThresholdFactor;
        collapse_threshold = ideal_edge_length * constants::kCollapseThresholdFactor;
        split_threshold_2 = split_threshold * split_threshold;
        collapse_threshold_2 = collapse_threshold * collapse_threshold;

        if (!is_quiet) {
            std::cout << "bbox_diag_length = " << bbox_diag_length << std::endl;
            std::cout << "ideal_edge_length = " << ideal_edge_length << std::endl;
            std::cout << "stage = " << stage << std::endl;
            std::cout << "eps_input = " << eps_input << std::endl;
            std::cout << "eps = " << eps << std::endl;
            std::cout << "eps_simplification = " << eps_simplification << std::endl;
            std::cout << "eps_coplanar = " << eps_coplanar << std::endl;
            std::cout << "dd = " << dd << std::endl;
            std::cout << "dd_simplification = " << dd_simplification << std::endl;
        }

        return true;
    }
};
}  // namespace floatTetWild