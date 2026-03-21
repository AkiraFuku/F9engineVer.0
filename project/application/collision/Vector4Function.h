#pragma once
#include "Vector2.h"
#include "Vector4.h"

Vector4 operator+(const Vector4& v1, const Vector4& v2);
Vector4 operator+=(Vector4& v1, const Vector4& v2);
Vector4 operator-(const Vector4& v1, const Vector4& v2);
Vector4 operator-=(Vector4& v1, const Vector4& v2);

float Length(const Vector2& v);
Vector2 Normalize(const Vector2& v);
