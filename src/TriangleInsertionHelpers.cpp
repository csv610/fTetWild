// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#include <floattetwild/TriangleInsertionHelpers.h>

#include <floattetwild/Error.h>
#include <floattetwild/LocalOperations.h>
#include <floattetwild/intersections.h>
#include <floattetwild/MeshIO.hpp>
#include <floattetwild/Predicates.hpp>

#include <igl/writeOFF.h>

floatTetWild::Vector3 floatTetWild::get_normal(const Vector3& a, const Vector3& b, const Vector3& c)
{ return ((b - c).cross(a - c)).normalized(); }

int floatTetWild::get_opp_t_id(int t_id, int j, const Mesh& mesh)
{
    std::vector<int> tmp;
    set_intersection(mesh.tet_vertices[mesh.tets[t_id][(j + 1) % 4]].conn_tets,
                     mesh.tet_vertices[mesh.tets[t_id][(j + 2) % 4]].conn_tets,
                     mesh.tet_vertices[mesh.tets[t_id][(j + 3) % 4]].conn_tets,
                     tmp);
    if (tmp.size() == 2)
        return tmp[0] == t_id ? tmp[1] : tmp[0];
    else
        return -1;
}

void floatTetWild::myassert(bool b, const std::string& s)
{
    if (b == false) {
        throw MeshError("Assertion failed: " + s, ErrorCode::UnexpectedState);
    }
}

void floatTetWild::check_track_surface_fs(
  Mesh&                                         mesh,
  std::vector<std::array<std::vector<int>, 4>>& track_surface_fs,
  const std::vector<Vector3>&                   input_vertices,
  const std::vector<Vector3i>&                  input_faces,
  const std::vector<int>&                       sorted_f_ids)
{ return; }

int floatTetWild::orient_rational(const Vector3_r& p1,
                                  const Vector3_r& p2,
                                  const Vector3_r& p3,
                                  const Vector3_r& p)
{
    auto              nv  = (p2 - p1).cross(p3 - p1);
    triwild::Rational res = nv.dot(p - p1);
    if (res == 0)
        return Predicates::ORI_ZERO;
    if (res < 0)
        return Predicates::ORI_POSITIVE;
    else
        return Predicates::ORI_NEGATIVE;
}