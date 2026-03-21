#include "Camera.h"
#include "MathFunction.h"
Camera::Camera()
    :transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
    , fovY(0.45f)
    , aspect(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight))
    , nearCrip(0.1f)
    , farCrip(100.0f)
    , worldMatrix(MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate))
    , viewMatrix(Inverse(worldMatrix))
    , projectionMatrix(MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip))
    , viewProtectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}
void Camera::Update() {

    worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);

}

void Camera::UpdateView()
{
    viewMatrix = Inverse(worldMatrix);
}

void Camera::UpdateViewProjection()
{
    projectionMatrix = MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip);
    viewProtectionMatrix = Multiply(viewMatrix, projectionMatrix);
}