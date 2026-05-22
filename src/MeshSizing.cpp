// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/MeshSizing.h>

#include <floattetwild/Parameters.h>
#include <floattetwild/EdgeCollapsing.h>
#include <floattetwild/EdgeSplitting.h>
#include <floattetwild/LocalOperations.h>
#include <floattetwild/MeshImprovement.h>
#include <floattetwild/Logger.hpp>

#include <geogram/points/kd_tree.h>

#include <cmath>
#include <queue>
#include <unordered_set>

bool floatTetWild::update_scaling_field(Mesh& mesh, Scalar max_energy)
{
    std::cout << "updating sclaing field ..." << std::endl;
    bool is_hit_min_edge_length = false;

    Scalar radius0 = mesh.params.ideal_edge_length * constants::kRadiusScaleFactor;

    static const Scalar stop_filter_energy = mesh.params.stop_energy * 0.8;

    Scalar filter_energy =
      max_energy / 100 > stop_filter_energy ? max_energy / 100 : stop_filter_energy;

    if (filter_energy > constants::kFilterEnergyThreshold) {
        filter_energy = 100;
    }

    std::cout << "filter_energy = " << filter_energy << std::endl;
    Scalar              recover = 1.5;
    std::vector<Scalar> scale_multipliers(mesh.tet_vertices.size(), recover);
    Scalar              refine_scale = 0.5;
    Scalar min_refine_scale = mesh.params.min_edge_len_rel / mesh.params.ideal_edge_length_rel;

    const int                     N = -int(std::log2(min_refine_scale) - 1);
    std::vector<std::vector<int>> v_ids(N, std::vector<int>());
    for (int i = 0; i < mesh.tet_vertices.size(); i++) {
        auto& v = mesh.tet_vertices[i];
        if (v.is_removed)
            continue;

        bool is_refine = false;
        for (int t_id : v.conn_tets) {
            if (mesh.tets[t_id].quality > filter_energy)
                is_refine = true;
        }
        if (!is_refine)
            continue;

        int n = -int(std::log2(v.sizing_scalar) - 0.5);
        if (n >= N)
            n = N - 1;
        v_ids[n].push_back(i);
    }

    for (int n = 0; n < N; n++) {
        if (v_ids[n].size() == 0)
            continue;

        Scalar radius = radius0 / std::pow(2, n);

        std::unordered_set<int> is_visited;
        std::queue<int>         v_queue;

        std::vector<double> pts;
        pts.reserve(v_ids[n].size() * 3);
        for (int i = 0; i < v_ids[n].size(); i++) {
            pts.push_back(mesh.tet_vertices[v_ids[n][i]].pos[0]);
            pts.push_back(mesh.tet_vertices[v_ids[n][i]].pos[1]);
            pts.push_back(mesh.tet_vertices[v_ids[n][i]].pos[2]);

            v_queue.push(v_ids[n][i]);
            is_visited.insert(v_ids[n][i]);
            scale_multipliers[v_ids[n][i]] = refine_scale;
        }
        GEO::NearestNeighborSearch_var nnsearch = GEO::NearestNeighborSearch::create(3, "BNN");
        nnsearch->set_points(int(v_ids[n].size()), pts.data());

        while (!v_queue.empty()) {
            int v_id = v_queue.front();
            v_queue.pop();

            for (int t_id : mesh.tet_vertices[v_id].conn_tets) {
                for (int j = 0; j < 4; j++) {
                    if (is_visited.find(mesh.tets[t_id][j]) != is_visited.end())
                        continue;
                    GEO::index_t _;
                    double       sq_dist;
                    const double p[3] = {mesh.tet_vertices[mesh.tets[t_id][j]].pos[0],
                                         mesh.tet_vertices[mesh.tets[t_id][j]].pos[1],
                                         mesh.tet_vertices[mesh.tets[t_id][j]].pos[2]};
                    nnsearch->get_nearest_neighbors(1, p, &_, &sq_dist);
                    Scalar dis = sqrt(sq_dist);

                    if (dis < radius) {
                        v_queue.push(mesh.tets[t_id][j]);
                        Scalar new_ss = (dis / radius) * (1 - refine_scale) + refine_scale;
                        if (new_ss < scale_multipliers[mesh.tets[t_id][j]])
                            scale_multipliers[mesh.tets[t_id][j]] = new_ss;
                    }
                    is_visited.insert(mesh.tets[t_id][j]);
                }
            }
        }
    }

    for (int i = 0; i < mesh.tet_vertices.size(); i++) {
        auto& v = mesh.tet_vertices[i];
        if (v.is_removed)
            continue;
        Scalar new_scale = v.sizing_scalar * scale_multipliers[i];
        if (new_scale > 1)
            v.sizing_scalar = 1;
        else if (new_scale < min_refine_scale) {
            is_hit_min_edge_length = true;
            v.sizing_scalar        = min_refine_scale;
        }
        else
            v.sizing_scalar = new_scale;
    }

    std::cout << "is_hit_min_edge_length = " << is_hit_min_edge_length << std::endl;
    return is_hit_min_edge_length;
}

void floatTetWild::apply_sizingfield(Mesh& mesh, AABBWrapper& tree)
{
    if (!mesh.params.apply_sizing_field)
        return;

    std::cout << "applying sizing field..." << std::endl;

    std::vector<double> pts;
    for (int i = 0; i < mesh.params.V_sizing_field.rows(); i++) {
        for (int j = 0; j < 3; j++)
            pts.push_back(mesh.params.V_sizing_field(i, j));
    }

    GEO::NearestNeighborSearch_var nnsearch = GEO::NearestNeighborSearch::create(3, "BNN");
    nnsearch->set_points(mesh.params.V_sizing_field.rows(), pts.data());

    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;

        const double p[3] = {v.pos[0], v.pos[1], v.pos[2]};
        GEO::index_t idx;
        double       sq_dist;
        nnsearch->get_nearest_neighbors(1, p, &idx, &sq_dist);
        v.sizing_scalar = mesh.params.values_sizing_field(idx);
    }
}

void floatTetWild::apply_coarsening(Mesh& mesh, AABBWrapper& tree)
{
    if (!mesh.params.coarsen)
        return;

    std::cout << "applying coarsening..." << std::endl;

    for (auto& v : mesh.tet_vertices) {
        if (v.is_removed)
            continue;
        v.sizing_scalar = 2.5;
    }
}