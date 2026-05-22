// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/MeshCleanup.h>

#include <floattetwild/Logger.hpp>

#include <igl/remove_duplicate_vertices.h>
#include <igl/unique_rows.h>

bool floatTetWild::remove_duplicates(std::vector<Vector3>&  input_vertices,
                                     std::vector<Vector3i>& input_faces,
                                     std::vector<int>&      input_tags,
                                     const Parameters&      params)
{
    MatrixXs        V_tmp(input_vertices.size(), 3), V_in;
    Eigen::MatrixXi F_tmp(input_faces.size(), 3), F_in;
    for (int i = 0; i < input_vertices.size(); i++)
        V_tmp.row(i) = input_vertices[i];
    for (int i = 0; i < input_faces.size(); i++)
        F_tmp.row(i) = input_faces[i];

    //
    Eigen::VectorXi IV, _;
    igl::remove_duplicate_vertices(
      V_tmp, F_tmp, SCALAR_ZERO * params.bbox_diag_length, V_in, IV, _, F_in);
    //
    for (int i = 0; i < F_in.rows(); i++) {
        int j_min = 0;
        for (int j = 1; j < 3; j++) {
            if (F_in(i, j) < F_in(i, j_min))
                j_min = j;
        }
        if (j_min == 0)
            continue;
        int v0_id = F_in(i, j_min);
        int v1_id = F_in(i, (j_min + 1) % 3);
        int v2_id = F_in(i, (j_min + 2) % 3);
        F_in.row(i) << v0_id, v1_id, v2_id;
    }
    F_tmp.resize(0, 0);
    Eigen::VectorXi IF;
    igl::unique_rows(F_in, F_tmp, IF, _);
    F_in                            = F_tmp;
    std::vector<int> old_input_tags = input_tags;
    input_tags.resize(IF.rows());
    for (int i = 0; i < IF.rows(); i++) {
        input_tags[i] = old_input_tags[IF(i)];
    }
    //
    if (V_in.rows() == 0 || F_in.rows() == 0)
        return false;

    logger().info("remove duplicates: ");
    logger().info("#v: {} -> {}", input_vertices.size(), V_in.rows());
    logger().info("#f: {} -> {}", input_faces.size(), F_in.rows());

    input_vertices.resize(V_in.rows());
    input_faces.clear();
    input_faces.reserve(F_in.rows());
    old_input_tags = input_tags;
    input_tags.clear();
    for (int i = 0; i < V_in.rows(); i++)
        input_vertices[i] = V_in.row(i);
    for (int i = 0; i < F_in.rows(); i++) {
        if (F_in(i, 0) == F_in(i, 1) || F_in(i, 0) == F_in(i, 2) || F_in(i, 2) == F_in(i, 1))
            continue;
        if (i > 0 && (F_in(i, 0) == F_in(i - 1, 0) && F_in(i, 1) == F_in(i - 1, 2) &&
                      F_in(i, 2) == F_in(i - 1, 1)))
            continue;
        Vector3 u    = V_in.row(F_in(i, 1)) - V_in.row(F_in(i, 0));
        Vector3 v    = V_in.row(F_in(i, 2)) - V_in.row(F_in(i, 0));
        Vector3 area = u.cross(v);
        if (area.norm() / 2 <= SCALAR_ZERO * params.bbox_diag_length)
            continue;
        input_faces.push_back(F_in.row(i));
        input_tags.push_back(old_input_tags[i]);
    }

    return true;
}