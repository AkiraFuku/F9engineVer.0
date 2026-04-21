#include "Camera.h"
#include "MathFunction.h"
#include "ImGuI.h"
#include "ImGuiManager.h"
Camera::Camera()
    :transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
    , fovY(0.45f)
    , aspect(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight))
    , nearCrip(0.1f)
    , farCrip(1000.0f)
    , worldMatrix(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
    , viewMatrix(Inverse(worldMatrix))
    , projectionMatrix(MakePerspectiveFovMatrix(fovY, aspect, nearCrip, farCrip))
    , viewProtectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}
void Camera::Update() {

    worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

#ifdef USE_IMGUI
    ImGui::Begin("camera");

    ImGui::DragFloat3("Rotate", &(transform_.rotate.x));
    ImGui::DragFloat3("scale", &(transform_.scale.x));
    ImGui::DragFloat3("translate", &(transform_.translate.x));
    ImGui::End();

#endif // USE_IMGUI



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