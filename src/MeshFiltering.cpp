// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/MeshFiltering.h>

#include <floattetwild/LocalOperations.h>
#include <floattetwild/MeshImprovement.h>
#include <floattetwild/MeshSurface.h>
#include <floattetwild/TriangleInsertionHelpers.h>
#include <floattetwild/Logger.hpp>

#include <igl/fast_winding_number.h>
#include <igl/winding_number.h>

void floatTetWild::filter_outside(Mesh& mesh, bool invert_faces)
{
    Eigen::MatrixXd C(mesh.get_t_num(), 3);
    C.setZero();
    int index = 0;
    for (size_t i = 0; i < mesh.tets.size(); i++) {
        if (mesh.tets[i].is_removed)
            continue;
        for (int j = 0; j < 4; j++)
            C.row(index) += mesh.tet_vertices[mesh.tets[i][j]].pos.cast<double>();
        C.row(index) /= 4.0;
        index++;
    }

    Eigen::Matrix<Scalar, Eigen::Dynamic, 3> V;
    Eigen::Matrix<int, Eigen::Dynamic, 3>    F;
    get_tracked_surface(mesh, V, F);
    Eigen::VectorXd W;
    if (invert_faces) {
        Eigen::Matrix<int, Eigen::Dynamic, 1> tmp = F.col(1);
        F.col(1)                                  = F.col(2).eval();
        F.col(2)                                  = tmp;
    }
    if (!mesh.params.use_general_wn)
        igl::fast_winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);
    else
        igl::winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);

    index                    = 0;
    int               n_tets = 0;
    std::vector<bool> old_flags(mesh.tets.size());
    for (int t_id = 0; t_id < mesh.tets.size(); ++t_id) {
        auto& t         = mesh.tets[t_id];
        old_flags[t_id] = t.is_removed;

        if (t.is_removed)
            continue;
        if (W(index) <= 0.5) {
            t.is_removed = true;
        }
        else
            n_tets++;
        index++;
    }

    if (n_tets <= 0) {
        if (invert_faces)
            logger().error("Empty mesh, problem with inverted faces");
        else {
            for (int t_id = 0; t_id < mesh.tets.size(); ++t_id) {
                auto& t      = mesh.tets[t_id];
                t.is_removed = old_flags[t_id];
            }
            logger().debug("Empty mesh trying to reverse the faces");
            filter_outside(mesh, true);
        }
    }

    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;
        bool is_remove = true;
        for (int t_id : v.conn_tets) {
            if (!mesh.tets[t_id].is_removed) {
                is_remove = false;
                break;
            }
        }
        v.is_removed = is_remove;
    }
}

void floatTetWild::filter_outside(Mesh&                        mesh,
                                  const std::vector<Vector3>&  input_vertices,
                                  const std::vector<Vector3i>& input_faces)
{
    Eigen::MatrixXd C(mesh.get_t_num(), 3);
    C.setZero();
    int index = 0;
    for (size_t i = 0; i < mesh.tets.size(); i++) {
        if (mesh.tets[i].is_removed)
            continue;
        for (int j = 0; j < 4; j++)
            C.row(index) += mesh.tet_vertices[mesh.tets[i][j]].pos.cast<double>();
        C.row(index) /= 4.0;
        index++;
    }

    Eigen::Matrix<Scalar, Eigen::Dynamic, 3> V(input_vertices.size(), 3);
    Eigen::Matrix<int, Eigen::Dynamic, 3>    F(input_faces.size(), 3);
    for (int i = 0; i < input_vertices.size(); i++)
        V.row(i) = input_vertices[i];
    for (int i = 0; i < input_faces.size(); i++)
        F.row(i) = input_faces[i];

    Eigen::VectorXd W;
    if (!mesh.params.use_general_wn)
        igl::fast_winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);
    else
        igl::winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);

    index      = 0;
    int n_tets = 0;
    for (int t_id = 0; t_id < mesh.tets.size(); ++t_id) {
        auto& t = mesh.tets[t_id];
        if (t.is_removed)
            continue;
        if (W(index) <= 0.5) {
            t.is_removed = true;
        }
        else
            n_tets++;
        index++;
    }

    if (n_tets <= 0)
        logger().error("Empty mesh");

    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;
        bool is_remove = true;
        for (int t_id : v.conn_tets) {
            if (!mesh.tets[t_id].is_removed) {
                is_remove = false;
                break;
            }
        }
        v.is_removed = is_remove;
    }
}

void floatTetWild::filter_outside_floodfill(Mesh& mesh, bool invert_faces)
{
    Eigen::MatrixXd C(mesh.get_t_num(), 3);
    C.setZero();
    int index = 0;
    for (size_t i = 0; i < mesh.tets.size(); i++) {
        if (mesh.tets[i].is_removed)
            continue;
        for (int j = 0; j < 4; j++)
            C.row(index) += mesh.tet_vertices[mesh.tets[i][j]].pos.cast<double>();
        C.row(index) /= 4.0;
        index++;
    }

    Eigen::Matrix<Scalar, Eigen::Dynamic, 3> V;
    Eigen::Matrix<int, Eigen::Dynamic, 3>    F;
    get_tracked_surface(mesh, V, F);
    Eigen::VectorXd W;
    if (invert_faces) {
        Eigen::Matrix<int, Eigen::Dynamic, 1> tmp = F.col(1);
        F.col(1)                                  = F.col(2).eval();
        F.col(2)                                  = tmp;
    }
    if (!mesh.params.use_general_wn)
        igl::fast_winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);
    else
        igl::winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);

    std::queue<int>   queue;
    std::vector<bool> is_visited(mesh.tets.size(), false);

    for (int i = 0; i < W.size(); i++) {
        if (W(i) > 0.5) {
            int cnt = 0;
            for (int t_id = 0; t_id < mesh.tets.size(); t_id++) {
                if (mesh.tets[t_id].is_removed)
                    continue;
                if (cnt == i) {
                    queue.push(t_id);
                    break;
                }
                cnt++;
            }
            break;
        }
    }

    while (!queue.empty()) {
        int t_id = queue.front();
        queue.pop();

        if (is_visited[t_id])
            continue;
        is_visited[t_id] = true;

        auto& t = mesh.tets[t_id];
        for (int j = 0; j < 4; j++) {
            int opp_t_id = get_opp_t_id(t_id, j, mesh);
            if (opp_t_id < 0)
                continue;
            if (is_visited[opp_t_id])
                continue;
            if (mesh.tets[opp_t_id].is_removed)
                continue;

            queue.push(opp_t_id);
        }
    }

    for (int t_id = 0; t_id < mesh.tets.size(); t_id++) {
        if (mesh.tets[t_id].is_removed)
            continue;
        if (!is_visited[t_id])
            mesh.tets[t_id].is_removed = true;
    }

    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;
        bool is_remove = true;
        for (int t_id : v.conn_tets) {
            if (!mesh.tets[t_id].is_removed) {
                is_remove = false;
                break;
            }
        }
        v.is_removed = is_remove;
    }
}

void floatTetWild::mark_outside(Mesh& mesh, bool invert_faces)
{
    Eigen::MatrixXd C(mesh.get_t_num(), 3);
    C.setZero();
    int index = 0;
    for (size_t i = 0; i < mesh.tets.size(); i++) {
        if (mesh.tets[i].is_removed)
            continue;
        for (int j = 0; j < 4; j++)
            C.row(index) += mesh.tet_vertices[mesh.tets[i][j]].pos.cast<double>();
        C.row(index) /= 4.0;
        index++;
    }

    Eigen::Matrix<Scalar, Eigen::Dynamic, 3> V;
    Eigen::Matrix<int, Eigen::Dynamic, 3>    F;
    get_tracked_surface(mesh, V, F);
    Eigen::VectorXd W;
    if (invert_faces) {
        Eigen::Matrix<int, Eigen::Dynamic, 1> tmp = F.col(1);
        F.col(1)                                  = F.col(2).eval();
        F.col(2)                                  = tmp;
    }
    if (!mesh.params.use_general_wn)
        igl::fast_winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);
    else
        igl::winding_number(Eigen::MatrixXd(V.cast<double>()), Eigen::MatrixXi(F), C, W);

    index = 0;
    for (int t_id = 0; t_id < mesh.tets.size(); ++t_id) {
        auto& t = mesh.tets[t_id];

        if (t.is_removed)
            continue;

        if (W(index) <= 0.5) {
            t.is_outside = true;
        }
        index++;
    }
}

void floatTetWild::untangle(Mesh& mesh)
{
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        if (t.quality > 0)
            continue;

        std::array<Vector3, 4> vs;
        for (int j = 0; j < 4; j++)
            vs[j] = mesh.tet_vertices[t[j]].pos;

        Scalar a = ((vs[1] - vs[0]).cross(vs[2] - vs[0])).dot(vs[3] - vs[0]);
        if (a > 0)
            continue;

        for (int j = 0; j < 4; j++) {
            mesh.tet_vertices[t[j]].is_outside = true;
        }
    }
}