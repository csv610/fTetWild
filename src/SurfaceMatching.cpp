// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/SurfaceMatching.h>

#include <floattetwild/LocalOperations.h>

#include <algorithm>
#include <random>

void floatTetWild::match_surface_fs(const Mesh&                                   mesh,
                                    const std::vector<Vector3>&                   input_vertices,
                                    const std::vector<Vector3i>&                  input_faces,
                                    std::vector<bool>&                            is_face_inserted,
                                    std::vector<std::array<std::vector<int>, 4>>& track_surface_fs)
{
    auto comp = [](const std::array<int, 4>& a, const std::array<int, 4>& b) {
        return std::tuple<int, int, int>(a[0], a[1], a[2]) <
               std::tuple<int, int, int>(b[0], b[1], b[2]);
    };

    std::vector<std::array<int, 4>> input_fs(input_faces.size());
    for (int i = 0; i < input_faces.size(); i++) {
        input_fs[i] = {{input_faces[i][0], input_faces[i][1], input_faces[i][2], i}};
        std::sort(input_fs[i].begin(), input_fs[i].begin() + 3);
    }
    std::sort(input_fs.begin(), input_fs.end(), comp);

    for (int i = 0; i < mesh.tets.size(); i++) {
        auto& t = mesh.tets[i];
        for (int j = 0; j < 4; j++) {
            std::array<int, 3> f = {{t[mod4(j + 1)], t[mod4(j + 2)], t[mod4(j + 3)]}};
            std::sort(f.begin(), f.end());
            auto bounds = std::equal_range(
              input_fs.begin(), input_fs.end(), std::array<int, 4>({{f[0], f[1], f[2], -1}}), comp);
            for (auto it = bounds.first; it != bounds.second; ++it) {
                int f_id               = (*it)[3];
                is_face_inserted[f_id] = true;
                track_surface_fs[i][j].push_back(f_id);
            }
        }
    }
}

void floatTetWild::sort_input_faces(const std::vector<Vector3>&  input_vertices,
                                    const std::vector<Vector3i>& input_faces,
                                    const Mesh&                  mesh,
                                    std::vector<int>&            sorted_f_ids)
{
    std::vector<Scalar> weights(input_faces.size());
    sorted_f_ids.resize(input_faces.size());
    for (int i = 0; i < input_faces.size(); i++) {
        sorted_f_ids[i] = i;

        Vector3 u  = input_vertices[input_faces[i][1]] - input_vertices[input_faces[i][0]];
        Vector3 v  = input_vertices[input_faces[i][2]] - input_vertices[input_faces[i][0]];
        weights[i] = u.cross(v).squaredNorm();
    }

    if (mesh.params.not_sort_input)
        return;
    std::random_device rd;
    std::mt19937       g(rd());

    std::shuffle(sorted_f_ids.begin(), sorted_f_ids.end(), g);
}