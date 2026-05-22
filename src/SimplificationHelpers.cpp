// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/SimplificationHelpers.h>

#include <floattetwild/LocalOperations.h>
#include <floattetwild/Logger.hpp>

#include <igl/writeOFF.h>

#ifdef FLOAT_TETWILD_USE_TBB
#include <oneapi/tbb/parallel_for.h>
#endif

floatTetWild::Scalar floatTetWild::get_angle_cos(const Vector3& p,
                                                 const Vector3& p1,
                                                 const Vector3& p2)
{
    Vector3 v1  = p1 - p;
    Vector3 v2  = p2 - p;
    Scalar  res = v1.dot(v2) / (v1.norm() * v2.norm());
    if (res > 1)
        return 1;
    if (res < -1)
        return -1;
    return res;
}

bool floatTetWild::is_out_envelope(const std::array<Vector3, 3>& vs,
                                   const AABBWrapper&            tree,
                                   const Parameters&             params)
{
#ifdef NEW_ENVELOPE
    return tree.is_out_sf_envelope_exact_simplify(vs);
#else
#ifdef STORE_SAMPLE_POINTS
    std::vector<GEO::vec3> ps;
    sample_triangle(vs, ps, params.dd_simplification);
    return tree.is_out_sf_envelope(ps, params.eps_2_simplification);
#else
    GEO::index_t prev_facet = GEO::NO_FACET;
    return sample_triangle_and_check_is_out(
      vs, params.dd_simplification, params.eps_2_simplification, tree, prev_facet);
#endif
#endif
}

void floatTetWild::check_surface(std::vector<Vector3>&    input_vertices,
                                 std::vector<Vector3i>&   input_faces,
                                 const std::vector<bool>& f_is_removed,
                                 const AABBWrapper&       tree,
                                 const Parameters&        params)
{
    std::cout << "checking surface" << std::endl;
    bool is_valid = true;
    for (int i = 0; i < input_faces.size(); i++) {
        if (f_is_removed[i])
            continue;
        std::vector<GEO::vec3> ps;
        sample_triangle({{input_vertices[input_faces[i][0]],
                          input_vertices[input_faces[i][1]],
                          input_vertices[input_faces[i][2]]}},
                        ps,
                        params.dd_simplification);
        Scalar dist = tree.dist_sf_envelope(ps, params.eps_2);
        if (dist > 0) {
            std::cout << "is_out_sf_envelope!!" << std::endl;
            is_valid = false;
            std::cout << input_faces[i][0] << " " << input_faces[i][1] << " " << input_faces[i][2]
                      << std::endl;
            std::cout << dist << std::endl;
        }
    }
}

void floatTetWild::output_component(const std::vector<Vector3>&  input_vertices,
                                    const std::vector<Vector3i>& input_faces,
                                    const std::vector<int>&      input_tags)
{
    return;

    Eigen::MatrixXd V(input_vertices.size(), 3);
    for (int i = 0; i < input_vertices.size(); i++)
        V.row(i) = input_vertices[i];

    const int       C = 2;
    Eigen::MatrixXi F(std::count(input_tags.begin(), input_tags.end(), C), 3);
    int             cnt = 0;
    for (int i = 0; i < input_tags.size(); i++) {
        if (input_tags[i] == C)
            F.row(cnt++) = input_faces[i];
    }

    igl::writeOFF("comp.off", V, F);
}