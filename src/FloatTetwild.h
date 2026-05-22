// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#pragma once

#include <floattetwild/Parameters.h>
#include <floattetwild/Logger.hpp>

namespace floatTetWild {

/// Main entry point for tetrahedral mesh generation.
/// @param[in] sf_mesh Input surface mesh
/// @param[in] params Configuration parameters
/// @param[out] VO Output vertices (Nx3 matrix)
/// @param[out] TO Output tetrahedra (Mx4 matrix)
/// @param[in] boolean_op Boolean operation (-1 for none, 0=diff, 1=union, 2=intersect)
/// @param[in] skip_simplify Skip simplification pass
/// @return 0 on success, non-zero on error
int tetrahedralization(GEO::Mesh&       sf_mesh,
                       Parameters       params,
                       Eigen::MatrixXd& VO,
                       Eigen::MatrixXi& TO,
                       int              boolean_op    = -1,
                       bool             skip_simplify = false);

}
