// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_TRIANGLE_INSERTION_HELPERS_H
#define FLOATTETWILD_TRIANGLE_INSERTION_HELPERS_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Mesh.hpp>
#include <floattetwild/Rational.h>

namespace floatTetWild {
    Vector3 get_normal(const Vector3& a, const Vector3& b, const Vector3& c);

    int get_opp_t_id(int t_id, int j, const Mesh& mesh);

    void myassert(bool b, const std::string& s);

    void check_track_surface_fs(Mesh &mesh, std::vector<std::array<std::vector<int>, 4>> &track_surface_fs,
                                const std::vector<Vector3> &input_vertices, const std::vector<Vector3i> &input_faces,
                                const std::vector<int> &sorted_f_ids);

    typedef Eigen::Matrix<triwild::Rational, 3, 1> Vector3_r;

    int orient_rational(const Vector3_r &p1, const Vector3_r &p2, const Vector3_r &p3, const Vector3_r &p);
}

#endif