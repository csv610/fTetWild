// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_EDGE_COLLAPSE_SIMPLIFY_H
#define FLOATTETWILD_EDGE_COLLAPSE_SIMPLIFY_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Parameters.h>
#include <floattetwild/AABBWrapper.h>

namespace floatTetWild {
    void collapsing(std::vector<Vector3>& input_vertices, std::vector<Vector3i>& input_faces, const AABBWrapper& sf_tree, const Parameters& params,
                   std::vector<bool>& is_v_removed, std::vector<bool>& is_f_removed, std::vector<std::unordered_set<int>>& conn_fs);
}

#endif