// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_SURFACE_MATCHING_H
#define FLOATTETWILD_SURFACE_MATCHING_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Mesh.hpp>

namespace floatTetWild {
    void match_surface_fs(const Mesh &mesh,
                          const std::vector<Vector3> &input_vertices, const std::vector<Vector3i> &input_faces,
                          std::vector<bool> &is_face_inserted,
                          std::vector<std::array<std::vector<int>, 4>> &track_surface_fs);

    void sort_input_faces(const std::vector<Vector3> &input_vertices, const std::vector<Vector3i> &input_faces,
                          const Mesh &mesh, std::vector<int> &sorted_f_ids);
}

#endif