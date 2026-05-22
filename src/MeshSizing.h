// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_MESH_SIZING_H
#define FLOATTETWILD_MESH_SIZING_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Mesh.hpp>
#include <floattetwild/AABBWrapper.h>

namespace floatTetWild {
    /// Update the sizing field based on element quality
    /// Uses kd-tree to propagate sizing requirements from high-quality elements
    /// @param mesh The mesh to update
    /// @param max_energy Maximum energy threshold for refinement
    /// @return true if minimum edge length was hit, false otherwise
    bool update_scaling_field(Mesh& mesh, Scalar max_energy);

    /// Apply sizing field from user-provided mesh
    /// @param mesh The mesh to apply sizing to
    /// @param tree AABB tree for spatial queries
    void apply_sizingfield(Mesh& mesh, const AABBWrapper& tree);

    /// Apply coarsening by increasing sizing scalar
    /// @param mesh The mesh to coarsen
    /// @param tree AABB tree for spatial queries
    void apply_coarsening(Mesh& mesh, const AABBWrapper& tree);
}

#endif