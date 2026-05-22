// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/EdgeSwapSimplify.h>

#include <floattetwild/LocalOperations.h>
#include <floattetwild/SimplificationHelpers.h>
#include <floattetwild/Logger.hpp>
#include <floattetwild/Mesh.hpp>

void floatTetWild::swapping(std::vector<Vector3>&                 input_vertices,
                            std::vector<Vector3i>&                input_faces,
                            const AABBWrapper&                    tree,
                            const Parameters&                     params,
                            std::vector<bool>&                    v_is_removed,
                            std::vector<bool>&                    f_is_removed,
                            std::vector<std::unordered_set<int>>& conn_fs)
{
    std::vector<std::array<int, 2>> edges;
    edges.reserve(input_faces.size() * 6);
    for (int i = 0; i < input_faces.size(); i++) {
        if (f_is_removed[i])
            continue;
        auto& f = input_faces[i];
        for (int j = 0; j < 3; j++) {
            std::array<int, 2> e = {{f[j], f[mod3(j + 1)]}};
            if (e[0] > e[1])
                std::swap(e[0], e[1]);
            edges.push_back(e);
        }
    }
    vector_unique(edges);

    std::priority_queue<ElementInQueue, std::vector<ElementInQueue>, cmp_l> sm_queue;
    for (auto& e : edges) {
        Scalar weight = (input_vertices[e[0]] - input_vertices[e[1]]).squaredNorm();
        sm_queue.push(ElementInQueue(e, weight));
        sm_queue.push(ElementInQueue(std::array<int, 2>({{e[1], e[0]}}), weight));
    }

    int cnt = 0;
    while (!sm_queue.empty()) {
        int v1_id = sm_queue.top().v_ids[0];
        int v2_id = sm_queue.top().v_ids[1];
        sm_queue.pop();

        std::vector<int> n12_f_ids;
        set_intersection(conn_fs[v1_id], conn_fs[v2_id], n12_f_ids);
        if (n12_f_ids.size() != 2)
            continue;

        std::array<int, 2> n_v_ids = {{-1, -1}};
        for (int j = 0; j < 3; j++) {
            if (n_v_ids[0] < 0 && input_faces[n12_f_ids[0]][j] != v1_id &&
                input_faces[n12_f_ids[0]][j] != v2_id)
                n_v_ids[0] = input_faces[n12_f_ids[0]][j];

            if (n_v_ids[1] < 0 && input_faces[n12_f_ids[1]][j] != v1_id &&
                input_faces[n12_f_ids[1]][j] != v2_id)
                n_v_ids[1] = input_faces[n12_f_ids[1]][j];
        }

        Scalar cos_a0 =
          get_angle_cos(input_vertices[n_v_ids[0]], input_vertices[v1_id], input_vertices[v2_id]);
        Scalar cos_a1 =
          get_angle_cos(input_vertices[n_v_ids[1]], input_vertices[v1_id], input_vertices[v2_id]);
        std::array<Vector3, 2> old_nvs;
        for (int f = 0; f < 2; f++) {
            auto& a    = input_vertices[input_faces[n12_f_ids[f]][0]];
            auto& b    = input_vertices[input_faces[n12_f_ids[f]][1]];
            auto& c    = input_vertices[input_faces[n12_f_ids[f]][2]];
            old_nvs[f] = ((b - c).cross(a - c)).normalized();
        }
        if (cos_a0 > -0.999) {
            if (old_nvs[0].dot(old_nvs[1]) < 1 - 1e-6)
                continue;
        }

        auto& old_nv  = cos_a1 < cos_a0 ? old_nvs[0] : old_nvs[1];
        bool  is_filp = false;
        for (int f_id : n12_f_ids) {
            auto& a = input_vertices[input_faces[f_id][0]];
            auto& b = input_vertices[input_faces[f_id][1]];
            auto& c = input_vertices[input_faces[f_id][2]];
            if (old_nv.dot(((b - c).cross(a - c)).normalized()) < 0) {
                is_filp = true;
                break;
            }
        }
        if (is_filp)
            continue;

        Scalar cos_a0_new = get_angle_cos(
          input_vertices[v1_id], input_vertices[n_v_ids[0]], input_vertices[n_v_ids[1]]);
        Scalar cos_a1_new = get_angle_cos(
          input_vertices[v2_id], input_vertices[n_v_ids[0]], input_vertices[n_v_ids[1]]);
        if (std::min(cos_a0_new, cos_a1_new) <= std::min(cos_a0, cos_a1))
            continue;

        if (is_out_envelope(
              {{input_vertices[v1_id], input_vertices[n_v_ids[0]], input_vertices[n_v_ids[1]]}},
              tree,
              params) ||
            is_out_envelope(
              {{input_vertices[v2_id], input_vertices[n_v_ids[0]], input_vertices[n_v_ids[1]]}},
              tree,
              params)) {
            continue;
        }

        for (int j = 0; j < 3; j++) {
            if (input_faces[n12_f_ids[0]][j] == v2_id)
                input_faces[n12_f_ids[0]][j] = n_v_ids[1];
            if (input_faces[n12_f_ids[1]][j] == v1_id)
                input_faces[n12_f_ids[1]][j] = n_v_ids[0];
        }
        conn_fs[v1_id].erase(n12_f_ids[1]);
        conn_fs[v2_id].erase(n12_f_ids[0]);
        conn_fs[n_v_ids[0]].insert(n12_f_ids[1]);
        conn_fs[n_v_ids[1]].insert(n12_f_ids[0]);
        cnt++;
    }

    logger().debug("{}  faces are swapped!!", cnt);
    return;
}