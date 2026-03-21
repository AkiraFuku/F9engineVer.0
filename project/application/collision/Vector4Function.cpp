#include "Vector4Function.h"

Vector4 operator+(const Vector4& v1, const Vector4& v2)
{
    Vector4 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;
    return result;
}

Vector4 operator+=(Vector4& v1, const Vector4& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}

Vector4 operator-(const Vector4& v1, const Vector4& v2)
{
    Vector4 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;
    return result;
}

Vector4 operator-=(Vector4& v1, const Vector4& v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    v1.w -= v2.w;
    return v1;
}

float Length(const Vector2& v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

Vector2 Normalize(const Vector2& v)
{
    float length = Length(v);

    return { v.x / length, v.y / length };
}

