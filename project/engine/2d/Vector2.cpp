#include "Vector2.h"

Vector2 Vector2::operator+(const Vector2& target)
{
    return { x + target.x,y + target.y };
}

Vector2 Vector2::operator-(const Vector2& target)
{
    return{ x - target.x,y - target.y };
}

Vector2 Vector2::operator*(const Vector2& target)
{
    return { x * target.x,y * target.y };
}

Vector2 Vector2::operator/(const Vector2& target)
{
    return { x / target.x,y / target.y };
}

Vector2 Vector2::operator+=(const Vector2& target)
{
    x += target.x;
    y += target.y;
    return *this;
}

Vector2 Vector2::operator-=(const Vector2& target)
{
    x -= target.x;
    y -= target.y;
    return *this;
}

Vector2 Vector2::operator*=(const Vector2& target)
{
    x *= target.x;
    y *= target.y;
    return *this;
}

Vector2 Vector2::operator/=(const Vector2& target)
{
    x /= target.x;
    y /= target.y;
    return *this;
}

Vector2 Vector2::operator*(const float& target)
{
    return{x*target,y*target};
}

Vector2 Vector2::operator*=(const float& target)
{
    x*=target;
    y*=target;
    return Vector2();
}

bool Vector2::operator!=(const Vector2& target)
{
    return!(*this == target);
}

bool Vector2::operator==(const Vector2& target)
{
    if (x != target.x)return false;
	if (y != target.y)return false;
    return true;
}

int2 int2::operator+(const int2& target)
{
    return { x + target.x,y + target.y };
}

int2 int2::operator-(const int2& target)
{
    return{ x - target.x,y - target.y };
}

int2 int2::operator*(const int2& target)
{
    return { x * target.x,y * target.y };
}

int2 int2::operator/(const int2& target)
{
    return { x / target.x,y / target.y };
}

int2 int2::operator+=(const int2& target)
{
    x += target.x;
    y += target.y;
    return *this;
}

int2 int2::operator-=(const int2& target)
{
    x -= target.x;
    y -= target.y;
    return *this;
}

int2 int2::operator*=(const int2& target)
{
    x *= target.x;
    y *= target.y;
    return *this;
}

int2 int2::operator/=(const int2& target)
{
    x /= target.x;
    y /= target.y;
    return *this;
}

int2 int2::operator*(const int& target)
{
    return{x*target,y*target};
}

int2 int2::operator*=(const int& target)
{
    x*=target;
    y*=target;
    return int2();
}

bool int2::operator!=(const int2& target)
{
    return!(*this == target);
}

bool int2::operator==(const int2& target)
{
    if (x != target.x)return false;
	if (y != target.y)return false;
    return true;
}
