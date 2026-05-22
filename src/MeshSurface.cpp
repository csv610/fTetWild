// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/MeshSurface.h>

#include <floattetwild/LocalOperations.h>
#include <floattetwild/MeshImprovement.h>
#include <floattetwild/bfs_orient.h>
#include <floattetwild/Logger.hpp>
#include <floattetwild/Predicates.hpp>

#include <igl/remove_duplicate_vertices.h>
#include <igl/writeSTL.h>

void floatTetWild::get_tracked_surface(const Mesh&                               mesh,
                                       Eigen::Matrix<Scalar, Eigen::Dynamic, 3>& V_sf,
                                       Eigen::Matrix<int, Eigen::Dynamic, 3>&    F_sf,
                                       int                                       c_id)
{
#define SF_CONDITION t.is_surface_fs[j] <= 0 && t.surface_tags[j] == c_id

    auto& tets         = mesh.tets;
    auto& tet_vertices = mesh.tet_vertices;

    int cnt = 0;
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (SF_CONDITION)
                cnt++;
        }
    }

    V_sf.resize(cnt * 3, 3);
    F_sf.resize(cnt, 3);
    cnt = 0;
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (SF_CONDITION) {
                for (int k = 0; k < 3; k++)
                    V_sf.row(cnt * 3 + k) = tet_vertices[t[mod4(j + k + 1)]].pos;
                if (Predicates::orient_3d(tet_vertices[t[mod4(j + 1)]].pos,
                                          tet_vertices[t[mod4(j + 2)]].pos,
                                          tet_vertices[t[mod4(j + 3)]].pos,
                                          tet_vertices[t[j]].pos) == Predicates::ORI_POSITIVE)
                    F_sf.row(cnt) << cnt * 3, cnt * 3 + 2, cnt * 3 + 1;
                else
                    F_sf.row(cnt) << cnt * 3, cnt * 3 + 1, cnt * 3 + 2;
                cnt++;
            }
        }
    }

    if (true || mesh.params.correct_surface_orientation) {
        Eigen::MatrixXd V;
        Eigen::MatrixXi F;
        Eigen::VectorXi _1, _2;
        igl::remove_duplicate_vertices(V_sf, F_sf, -1, V, _1, _2, F);
        V_sf = V;
        F_sf.resize(0, 3);
        bfs_orient(F, F_sf, _1);
    }
    igl::writeSTL(
      mesh.params.output_path + "_" + mesh.params.postfix + "_tracked_surface.stl", V_sf, F_sf);
}

void floatTetWild::correct_tracked_surface_orientation(Mesh& mesh, AABBWrapper& tree)
{
    std::vector<std::array<bool, 4>> is_visited(mesh.tets.size(), {{false, false, false, false}});
    for (int t_id = 0; t_id < mesh.tets.size(); t_id++) {
        auto& t = mesh.tets[t_id];
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (t.is_surface_fs[j] == NOT_SURFACE || is_visited[t_id][j])
                continue;
        }
    }
}

void floatTetWild::get_surface(const Mesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F)
{
    int cnt = 0;
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (t.is_surface_fs[j] <= 0)
                cnt++;
        }
    }

    V.resize(cnt * 3, 3);
    F.resize(cnt, 3);
    cnt = 0;
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (t.is_surface_fs[j] <= 0) {
                for (int k = 0; k < 3; k++)
                    V.row(cnt * 3 + k) = mesh.tet_vertices[t[mod4(j + k + 1)]].pos;
                F.row(cnt) << cnt * 3, cnt * 3 + 1, cnt * 3 + 2;
                cnt++;
            }
        }
    }

    Eigen::MatrixXd V_tmp;
    Eigen::MatrixXi F_tmp;
    Eigen::VectorXi _1, _2;
    igl::remove_duplicate_vertices(V, F, -1, V_tmp, _1, _2, F);
    V = V_tmp;
    F = F_tmp;
}

void floatTetWild::manifold_surface(const Mesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F)
{
    get_surface(mesh, V, F);

    std::vector<std::array<int, 2>> edges;
    for (int i = 0; i < F.rows(); i++) {
        for (int j = 0; j < 3; j++) {
            std::array<int, 2> e = {{F(i, j), F(i, (j + 1) % 3)}};
            if (e[0] > e[1])
                std::swap(e[0], e[1]);
            edges.push_back(e);
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

    std::unordered_map<int, std::vector<int>> v_to_edges;
    for (int i = 0; i < edges.size(); i++) {
        v_to_edges[edges[i][0]].push_back(i);
        v_to_edges[edges[i][1]].push_back(i);
    }

    std::vector<bool> is_removed_face(F.rows(), false);
    for (int i = 0; i < edges.size(); i++) {
        int v0 = edges[i][0];
        int v1 = edges[i][1];
        if (v_to_edges[v0].size() != 2 || v_to_edges[v1].size() != 2) {
            for (int f_id = 0; f_id < F.rows(); f_id++) {
                for (int j = 0; j < 3; j++) {
                    if ((F(f_id, j) == v0 && F(f_id, (j + 1) % 3) == v1) ||
                        (F(f_id, j) == v1 && F(f_id, (j + 1) % 3) == v0)) {
                        is_removed_face[f_id] = true;
                    }
                }
            }
        }
    }

    int cnt = 0;
    for (int i = 0; i < is_removed_face.size(); i++) {
        if (!is_removed_face[i])
            cnt++;
    }

    Eigen::MatrixXi F_new(cnt, 3);
    cnt = 0;
    for (int i = 0; i < F.rows(); i++) {
        if (!is_removed_face[i]) {
            F_new.row(cnt) = F.row(i);
            cnt++;
        }
    }
    F = F_new;
}

void floatTetWild::manifold_edges(Mesh& mesh)
{
    std::vector<std::array<int, 2>> edges;
    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (t.is_surface_fs[j] <= 0) {
                for (int k = 0; k < 3; k++) {
                    std::array<int, 2> e = {{t[mod4(j + k + 1)], t[mod4(j + k + 2)]}};
                    if (e[0] > e[1])
                        std::swap(e[0], e[1]);
                    edges.push_back(e);
                }
            }
        }
    }
    std::sort(edges.begin(), edges.end());

    std::vector<int> edge_cnt(edges.size(), 0);
    for (int i = 0; i < edges.size();) {
        int j = i + 1;
        while (j < edges.size() && edges[j] == edges[i]) {
            j++;
        }
        for (int k = i; k < j; k++)
            edge_cnt[k] = j - i;
        i = j;
    }

    std::vector<bool> is_removed_v(mesh.tet_vertices.size(), false);
    for (int i = 0; i < edges.size(); i++) {
        if (edge_cnt[i] != 2) {
            is_removed_v[edges[i][0]] = true;
            is_removed_v[edges[i][1]] = true;
        }
    }

    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (is_removed_v[t[j]])
                t.is_surface_fs[j] = NOT_SURFACE;
        }
    }
}

void floatTetWild::manifold_vertices(Mesh& mesh)
{
    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;
        v.is_on_boundary = false;
    }

    for (auto& t : mesh.tets) {
        if (t.is_removed)
            continue;
        for (int j = 0; j < 4; j++) {
            if (t.is_surface_fs[j] <= 0) {
                for (int k = 0; k < 3; k++)
                    mesh.tet_vertices[t[mod4(j + k + 1)]].is_on_boundary = true;
            }
        }
    }
}

void floatTetWild::output_surface(const Mesh& mesh, const std::string& filename)
{
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    get_surface(mesh, V, F);
    igl::writeSTL(filename, V, F);
}