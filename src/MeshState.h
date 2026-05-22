// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_MESH_STATE_H
#define FLOATTETWILD_MESH_STATE_H

#include <floattetwild/Types.hpp>
#include <floattetwild/Mesh.hpp>

#include <memory>
#include <vector>
#include <stdexcept>

namespace floatTetWild {

class MeshStateError : public std::runtime_error {
public:
    explicit MeshStateError(const std::string& message)
        : std::runtime_error(message) {}
};

class ScopedMeshState {
public:
    explicit ScopedMeshState(Mesh& mesh)
        : mesh_(mesh)
        , original_covered_tets_()
    {}

    ~ScopedMeshState() {
        restore();
    }

    ScopedMeshState(const ScopedMeshState&) = delete;
    ScopedMeshState& operator=(const ScopedMeshState&) = delete;

    ScopedMeshState(ScopedMeshState&&) = default;
    ScopedMeshState& operator=(ScopedMeshState&&) = default;

    void record_covered_tet(int tet_id, int face_id) {
        original_covered_tets_.push_back({tet_id, face_id});
    }

    void clear_covered_tets() {
        original_covered_tets_.clear();
    }

    const std::vector<std::array<int, 3>>& get_covered_tets() const {
        return original_covered_tets_;
    }

private:
    Mesh& mesh_;
    std::vector<std::array<int, 3>> original_covered_tets_;
};

class MeshStateManager {
public:
    explicit MeshStateManager(Mesh& mesh)
        : mesh_(mesh)
        , current_state_(nullptr)
    {}

    MeshStateManager(const MeshStateManager&) = delete;
    MeshStateManager& operator=(const MeshStateManager&) = delete;

    std::unique_ptr<ScopedMeshState> create_state() {
        return std::make_unique<ScopedMeshState>(mesh_);
    }

    void record_tet_face(int tet_id, int face_id) {
        if (current_state_) {
            current_state_->record_covered_tet(tet_id, face_id);
        }
    }

    void set_current_state(std::unique_ptr<ScopedMeshState> state) {
        current_state_ = std::move(state);
    }

    void clear_current_state() {
        current_state_.reset();
    }

private:
    Mesh& mesh_;
    std::unique_ptr<ScopedMeshState> current_state_;
};

}  // namespace floatTetWild

#endif