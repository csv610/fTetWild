// This file is part of fTetWild, a software for generating tetrahedral meshes.
//
// Copyright (C) 2019 Yixin Hu <yixin.hu@nyu.edu>
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef FLOATTETWILD_ERROR_H
#define FLOATTETWILD_ERROR_H

#include <stdexcept>
#include <string>
#include <sstream>

namespace floatTetWild {

enum class ErrorCode {
    Success = 0,
    MeshEmpty = 1,
    InvalidInput = 2,
    EnvelopeViolation = 3,
    InvertedElement = 4,
    QualityFailure = 5,
    AllocationFailure = 6,
    UnexpectedState = 7
};

class MeshError : public std::runtime_error {
public:
    explicit MeshError(const std::string& message, ErrorCode code = ErrorCode::UnexpectedState)
        : std::runtime_error(message)
        , code_(code)
    {}

    explicit MeshError(const std::string& message, ErrorCode code, const std::string& context)
        : std::runtime_error(message + " [Context: " + context + "]")
        , code_(code)
    {}

    ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
};

class EnvelopeError : public MeshError {
public:
    explicit EnvelopeError(const std::string& message)
        : MeshError(message, ErrorCode::EnvelopeViolation)
    {}
};

class QualityError : public MeshError {
public:
    explicit QualityError(const std::string& message)
        : MeshError(message, ErrorCode::QualityFailure)
    {}
};

class InputError : public MeshError {
public:
    explicit InputError(const std::string& message)
        : MeshError(message, ErrorCode::InvalidInput)
    {}
};

inline void throw_if(bool condition, const std::string& message, ErrorCode code = ErrorCode::UnexpectedState) {
    if (condition) {
        throw MeshError(message, code);
    }
}

inline void throwEnvelopeError(bool condition, const std::string& message) {
    if (condition) {
        throw EnvelopeError(message);
    }
}

inline void throwQualityError(bool condition, const std::string& message) {
    if (condition) {
        throw QualityError(message);
    }
}

inline void throwInputError(bool condition, const std::string& message) {
    if (condition) {
        throw InputError(message);
    }
}

}  // namespace floatTetWild

#endif