// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/Simplification.h>

#include <floattetwild/Logger.hpp>

#include <igl/Timer.h>
#include <igl/writeOFF.h>

void floatTetWild::simplify(std::vector<Vector3>&  input_vertices,
                            std::vector<Vector3i>& input_faces,
                            std::vector<int>&      input_tags,
                            const AABBWrapper&     tree,
                            const Parameters&     params,
                            bool                   skip_simplify)
{
    remove_duplicates(input_vertices, input_faces, input_tags, params);
    if (skip_simplify)
        return;

    std::vector<bool>                    v_is_removed(input_vertices.size(), false);
    std::vector<bool>                    f_is_removed(input_faces.size(), false);
    std::vector<std::unordered_set<int>> conn_fs(input_vertices.size());
    for (int i = 0; i < input_faces.size(); i++) {
        for (int j = 0; j < 3; j++)
            conn_fs[input_faces[i][j]].insert(i);
    }

    igl::Timer timer;
    timer.start();
    collapsing(input_vertices, input_faces, tree, params, v_is_removed, f_is_removed, conn_fs);
    std::cout << "collapsing " << timer.getElapsedTime() << std::endl;

    timer.start();
    swapping(input_vertices, input_faces, tree, params, v_is_removed, f_is_removed, conn_fs);
    std::cout << "swapping " << timer.getElapsedTime() << std::endl;

    std::vector<int> map_v_ids(input_vertices.size(), -1);
    int              cnt = 0;
    for (int i = 0; i < input_vertices.size(); i++) {
        if (v_is_removed[i] || conn_fs[i].empty())
            continue;
        map_v_ids[i] = cnt;
        cnt++;
    }

    std::vector<Vector3> new_input_vertices(cnt);
    cnt = 0;
    for (int i = 0; i < input_vertices.size(); i++) {
        if (v_is_removed[i] || conn_fs[i].empty())
            continue;
        new_input_vertices[cnt++] = input_vertices[i];
    }
    input_vertices = new_input_vertices;

    cnt = 0;
    for (int i = 0; i < input_faces.size(); i++) {
        if (f_is_removed[i])
            continue;
        for (int j = 0; j < 3; j++)
            input_faces[i][j] = map_v_ids[input_faces[i][j]];
        cnt++;
    }

    std::vector<Vector3i> new_input_faces(cnt);
    std::vector<int>      new_input_tags(cnt);
    cnt = 0;
    for (int i = 0; i < input_faces.size(); i++) {
        if (f_is_removed[i])
            continue;
        new_input_faces[cnt] = input_faces[i];
        new_input_tags[cnt]  = input_tags[i];
        cnt++;
    }
    input_faces = new_input_faces;
    input_tags  = new_input_tags;

    remove_duplicates(input_vertices, input_faces, input_tags, params);

    logger().info("#v = {}", input_vertices.size());
    logger().info("#f = {}", input_faces.size());

    if (params.log_level < 3) {
        Eigen::MatrixXd V(input_vertices.size(), 3);
        Eigen::MatrixXi F(input_faces.size(), 3);
        for (int i = 0; i < input_vertices.size(); i++) {
            V.row(i) = input_vertices[i];
        }
        for (int i = 0; i < input_faces.size(); i++) {
            F.row(i) = input_faces[i];
        }
        if (!params.output_path.empty()) {
            igl::writeOFF(params.output_path + "_" + params.postfix + "_simplify.off", V, F);
        }
    }
}