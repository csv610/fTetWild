// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/EdgeCollapseSimplify.h>

#include <floattetwild/LocalOperations.h>
#include <floattetwild/SimplificationHelpers.h>
#include <floattetwild/Logger.hpp>
#include <floattetwild/Mesh.hpp>

#include <igl/Timer.h>

#ifdef FLOAT_TETWILD_USE_TBB
#include <oneapi/tbb/concurrent_unordered_set.h>
#include <oneapi/tbb/concurrent_vector.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_sort.h>
#endif

void floatTetWild::collapsing(std::vector<Vector3>&                 input_vertices,
                              std::vector<Vector3i>&                input_faces,
                              const AABBWrapper&                    tree,
                              const Parameters&                     params,
                              std::vector<bool>&                    v_is_removed,
                              std::vector<bool>&                    f_is_removed,
                              std::vector<std::unordered_set<int>>& conn_fs)
{
#ifdef FLOAT_TETWILD_USE_TBB
    std::vector<std::array<int, 2>>            edges;
    tbb::concurrent_vector<std::array<int, 2>> edges_tbb;

    const auto build_edges = [&]() {
        edges.clear();
        edges.reserve(input_faces.size() * 3);

        edges_tbb.clear();
        edges_tbb.reserve(input_faces.size() * 3);

        tbb::parallel_for(size_t(0), input_faces.size(), [&](size_t f_id) {
            if (f_is_removed[f_id])
                return;

            for (int j = 0; j < 3; j++) {
                std::array<int, 2> e = {{input_faces[f_id][j], input_faces[f_id][mod3(j + 1)]}};
                if (e[0] > e[1])
                    std::swap(e[0], e[1]);
                edges_tbb.push_back(e);
            }
        });

        edges.reserve(edges_tbb.size());
        edges.insert(edges.end(), edges_tbb.begin(), edges_tbb.end());
        assert(edges_tbb.size() == edges.size());
        tbb::parallel_sort(edges.begin(), edges.end());

        edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    };
#else
    std::vector<std::array<int, 2>> edges;
    edges.clear();
    edges.reserve(input_faces.size() * 3);
    for (size_t f_id = 0; f_id < input_faces.size(); ++f_id) {
        if (f_is_removed[f_id])
            continue;

        const auto& f = input_faces[f_id];
        for (int j = 0; j < 3; j++) {
            std::array<int, 2> e = {{f[j], f[mod3(j + 1)]}};
            if (e[0] > e[1])
                std::swap(e[0], e[1]);
            edges.push_back(e);
        }
    }
    vector_unique(edges);

    std::priority_queue<ElementInQueue, std::vector<ElementInQueue>, cmp_s> sm_queue;
    for (auto& e : edges) {
        Scalar weight = (input_vertices[e[0]] - input_vertices[e[1]]).squaredNorm();
        sm_queue.push(ElementInQueue(e, weight));
        sm_queue.push(ElementInQueue(std::array<int, 2>({{e[1], e[0]}}), weight));
    }
#endif

    auto is_onering_clean = [&](int v_id) {
        std::vector<int> v_ids;
        v_ids.reserve(conn_fs[v_id].size() * 2);
        for (const auto& f_id : conn_fs[v_id]) {
            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] != v_id)
                    v_ids.push_back(input_faces[f_id][j]);
            }
        }
        std::sort(v_ids.begin(), v_ids.end());

        if (v_ids.size() % 2 != 0)
            return false;
        for (int i = 0; i < v_ids.size(); i += 2) {
            if (v_ids[i] != v_ids[i + 1])
                return false;
        }

        return true;
    };

    static const int SUC        = 1;
    static const int FAIL_CLEAN = 0;
    static const int FAIL_FLIP  = -1;
    static const int FAIL_ENV   = -2;

    auto remove_an_edge = [&](int v1_id, int v2_id, const std::vector<int>& n12_f_ids) {
        if (!is_onering_clean(v1_id) || !is_onering_clean(v2_id))
            return FAIL_CLEAN;

        std::vector<int> new_f_ids;
        for (int f_id : conn_fs[v1_id]) {
            if (f_id != n12_f_ids[0] && f_id != n12_f_ids[1])
                new_f_ids.push_back(f_id);
        }
        for (int f_id : conn_fs[v2_id]) {
            if (f_id != n12_f_ids[0] && f_id != n12_f_ids[1])
                new_f_ids.push_back(f_id);
        }
        vector_unique(new_f_ids);

        Vector3 p = (input_vertices[v1_id] + input_vertices[v2_id]) / 2;
        tree.project_to_sf(p);

        for (int f_id : new_f_ids) {
            Vector3 old_nv =
              (input_vertices[input_faces[f_id][1]] - input_vertices[input_faces[f_id][2]])
                .cross(input_vertices[input_faces[f_id][0]] - input_vertices[input_faces[f_id][2]]);

            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] == v1_id || input_faces[f_id][j] == v2_id) {
                    Vector3 new_nv = (input_vertices[input_faces[f_id][mod3(j + 1)]] -
                                      input_vertices[input_faces[f_id][mod3(j + 2)]])
                                       .cross(p - input_vertices[input_faces[f_id][mod3(j + 2)]]);
                    if (old_nv.dot(new_nv) <= 0)
                        return FAIL_FLIP;
                    if (new_nv.norm() / 2 <= SCALAR_ZERO_2)
                        return FAIL_FLIP;
                    break;
                }
            }
        }

        for (int f_id : new_f_ids) {
            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] == v1_id || input_faces[f_id][j] == v2_id) {
                    const std::array<Vector3, 3> tri = {
                      {p,
                       input_vertices[input_faces[f_id][mod3(j + 1)]],
                       input_vertices[input_faces[f_id][mod3(j + 2)]]}};
                    if (is_out_envelope(tri, tree, params))
                        return FAIL_ENV;
                    break;
                }
            }
        }

        std::vector<int> n_v_ids;
        for (int f_id : new_f_ids) {
            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] != v1_id && input_faces[f_id][j] != v2_id)
                    n_v_ids.push_back(input_faces[f_id][j]);
            }
        }
        vector_unique(n_v_ids);

        v_is_removed[v1_id]   = true;
        input_vertices[v2_id] = p;
        for (int f_id : n12_f_ids) {
            f_is_removed[f_id] = true;
#ifndef FLOAT_TETWILD_USE_TBB
            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] != v1_id) {
                    conn_fs[input_faces[f_id][j]].erase(f_id);
                }
            }
#endif
        }
        for (int f_id : conn_fs[v1_id]) {
            if (f_is_removed[f_id])
                continue;
            conn_fs[v2_id].insert(f_id);
            for (int j = 0; j < 3; j++) {
                if (input_faces[f_id][j] == v1_id)
                    input_faces[f_id][j] = v2_id;
            }
        }

#ifndef FLOAT_TETWILD_USE_TBB
        for (int v_id : n_v_ids) {
            double weight = (input_vertices[v2_id] - input_vertices[v_id]).squaredNorm();
            sm_queue.push(ElementInQueue(std::array<int, 2>({{v2_id, v_id}}), weight));
            sm_queue.push(ElementInQueue(std::array<int, 2>({{v_id, v2_id}}), weight));
        }
#endif
        return SUC;
    };

#ifdef FLOAT_TETWILD_USE_TBB
    std::atomic<int> cnt(0);
    int              cnt_suc = 0;

    const int stopping = input_vertices.size() / 10000.;

    std::vector<int> safe_set;
    do {
        build_edges();
        Mesh::one_ring_edge_set(
          edges, v_is_removed, f_is_removed, conn_fs, input_vertices, safe_set);
        cnt = 0;

        tbb::parallel_for(size_t(0), safe_set.size(), [&](size_t i) {
            std::array<int, 2>& v_ids = edges[safe_set[i]];

            std::vector<int> n12_f_ids;
            set_intersection(conn_fs[v_ids[0]], conn_fs[v_ids[1]], n12_f_ids);

            if (n12_f_ids.size() != 2)
                return;

            int res = remove_an_edge(v_ids[0], v_ids[1], n12_f_ids);
            if (res == SUC)
                cnt++;
        });

        tbb::parallel_for(size_t(0), conn_fs.size(), [&](size_t i) {
            if (v_is_removed[i])
                return;
            std::vector<int> r_f_ids;
            for (int f_id : conn_fs[i]) {
                if (f_is_removed[f_id])
                    r_f_ids.push_back(f_id);
            }
            for (int f_id : r_f_ids)
                conn_fs[i].erase(f_id);
        });

        cnt_suc += cnt;
    } while (cnt > stopping);

#else
    int cnt_suc    = 0;
    int fail_clean = 0;
    int fail_flip  = 0;
    int fail_env   = 0;

    while (!sm_queue.empty()) {
        std::array<int, 2> v_ids      = sm_queue.top().v_ids;
        Scalar             old_weight = sm_queue.top().weight;
        sm_queue.pop();

        if (v_is_removed[v_ids[0]] || v_is_removed[v_ids[1]])
            continue;
        if (old_weight != (input_vertices[v_ids[0]] - input_vertices[v_ids[1]]).squaredNorm())
            continue;

        std::vector<int> n12_f_ids;
        set_intersection(conn_fs[v_ids[0]], conn_fs[v_ids[1]], n12_f_ids);
        if (n12_f_ids.size() != 2)
            continue;

        int res = remove_an_edge(v_ids[0], v_ids[1], n12_f_ids);
        if (res == SUC)
            cnt_suc++;
    }
#endif

    logger().debug("{}  faces are collapsed!!", cnt_suc);
}