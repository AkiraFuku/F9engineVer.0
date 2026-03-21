#include "DebugCameraFunction.h"

Matrix4x4 MakeIdentity4x4()
{
        return
        { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f };
}

Matrix4x4 MakeLookAtMatrix(const Vector3& eye, const Vector3& target, const Vector3& up)
{
    Vector3 zAxis = Normalize(Subtract(target, eye));   // 前方向
    Vector3 xAxis = Normalize(Cross(up, zAxis));         // 右方向
    Vector3 yAxis = Cross(zAxis, xAxis);                 // 上方向

    Matrix4x4 view{};
    view.m[0][0] = xAxis.x; view.m[0][1] = yAxis.x; view.m[0][2] = zAxis.x; view.m[0][3] = 0.0f;
    view.m[1][0] = xAxis.y; view.m[1][1] = yAxis.y; view.m[1][2] = zAxis.y; view.m[1][3] = 0.0f;
    view.m[2][0] = xAxis.z; view.m[2][1] = yAxis.z; view.m[2][2] = zAxis.z; view.m[2][3] = 0.0f;
    view.m[3][0] = -Dot(xAxis, eye); view.m[3][1] = -Dot(yAxis, eye); view.m[3][2] = -Dot(zAxis, eye); view.m[3][3] = 1.0f;
    return view;
}