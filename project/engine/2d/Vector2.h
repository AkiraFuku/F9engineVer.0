#pragma once
struct Vector2{
    float x, y;

    Vector2 operator+(const Vector2& target);
    Vector2 operator-(const Vector2& target);
    Vector2 operator*(const Vector2& target);
    Vector2 operator/(const Vector2& target);
    Vector2 operator+=(const Vector2& target);
    Vector2 operator-=(const Vector2& target);
    Vector2 operator*=(const Vector2& target);
    Vector2 operator/=(const Vector2& target);
    Vector2 operator*(const float& target);
    Vector2 operator*=(const float& target);
    bool operator==(const Vector2& target);
    bool operator!=(const Vector2& target);
};
struct Gauge {
    int32_t value;  
    int32_t max;
};