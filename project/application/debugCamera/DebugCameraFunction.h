#pragma once
#include "mathFunction.h"
#include "numbers"
#include <algorithm>
#include "Vector4.h"

static const float pi = std::numbers::pi_v<float>;

// 単位行列の作成
Matrix4x4 MakeIdentity4x4();

Matrix4x4 MakeLookAtMatrix(const Vector3& eye, const Vector3& target, const Vector3& up);