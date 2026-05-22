// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_MESH_SURFACE_H
#define FLOATTETWILD_MESH_SURFACE_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Mesh.hpp>
#include <floattetwild/AABBWrapper.h>

namespace floatTetWild {
    /// Get tracked surface faces from mesh
    /// @param mesh Source mesh
    /// @param[out] V_sf Output vertices (3 per face)
    /// @param[out] F_sf Output face indices
    /// @param c_id Component ID to filter by (0 for all)
    void get_tracked_surface(const Mesh& mesh,
                             Eigen::Matrix<Scalar, Eigen::Dynamic, 3>& V_sf,
                             Eigen::Matrix<int, Eigen::Dynamic, 3>& F_sf,
                             int c_id = 0);

    /// Correct surface orientation using BFS
    /// @param mesh Mesh to correct
    /// @param tree AABB tree for spatial queries
    void correct_tracked_surface_orientation(Mesh& mesh, const AABBWrapper& tree);

    /// Extract surface faces from mesh
    /// @param mesh Source mesh
    /// @param[out] V Output vertices
    /// @param[out] F Output faces
    void get_surface(const Mesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);

    /// Extract manifold surface (remove non-manifold edges/vertices)
    /// @param mesh Source mesh
    /// @param[out] V Output vertices
    /// @param[out] F Output faces (manifold)
    void manifold_surface(const Mesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F);

    /// Mark non-manifold edges as not on surface
    /// @param mesh Mesh to process
    void manifold_edges(Mesh& mesh);

    /// Mark boundary vertices
    /// @param mesh Mesh to process
    void manifold_vertices(Mesh& mesh);

    /// Output surface to STL file
    /// @param mesh Source mesh
    /// @param filename Output filename
    void output_surface(const Mesh& mesh, const std::string& filename);
}

#endif