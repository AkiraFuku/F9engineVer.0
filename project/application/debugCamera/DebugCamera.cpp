#include "DebugCamera.h"
#include "Input.h"
#include "ImGuiManager.h"

void DebugCamera::Initialize()
{

	viewMatrix_ = MakeIdentity4x4();
	projectionMatrix_ = MakeIdentity4x4();
}

void DebugCamera::Update(EulerTransform originCamera)
{
#ifdef _DEBUG

	input_ = Input::GetInstance();

	Vector3 move = { 0.0f, 0.0f, 0.0f };

	EulerTransform camera = { 0.0f, 0.0f, 0.0f };

	/*if (input_->PushKey(DIK_RIGHT))
	{
		debugCamera_.rotate.y += 0.5f / 180.0f * pi;
	}
	else if (input_->PushKey(DIK_LEFT))
	{
		debugCamera_.rotate.y -= 0.5f / 180.0f * pi;
	}

	if (input_->PushKey(DIK_UP))
	{
		debugCamera_.rotate.x += 0.5f / 180.0f * pi;
	}
	else if (input_->PushKey(DIK_DOWN))
	{
		debugCamera_.rotate.x -= 0.5f / 180.0f * pi;
	}

	camera.rotate.x = debugCamera_.rotate.x + originCamera.rotate.x;
	camera.rotate.y = debugCamera_.rotate.y + originCamera.rotate.y;
	camera.rotate.z = debugCamera_.rotate.z + originCamera.rotate.z;

	if (input_->PushKey(DIK_W))
	{
		const float speed = 0.5f;
		move.z = speed;
	}
	else if (input_->PushKey(DIK_S))
	{
		const float speed = -0.5f;
		move.z = speed;
	}

	if (input_->PushKey(DIK_A))
	{
		const float speed = 0.5f;
		move.x = speed;
	}
	else if (input_->PushKey(DIK_D))
	{
		const float speed = -0.5f;
		move.x = speed;
	}

	if (input_->PushKey(DIK_R))
	{
		const float speed = 0.5f;
		move.y = speed;
	}
	else if (input_->PushKey(DIK_F))
	{
		const float speed = -0.5f;
		move.y = speed;
	}*/

	ImGui::Begin("DebugCameraWindow");

	if (input_->PushMouseDown(Input::MouseButton::Middle) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		// ウィンドウ上ではない領域でのドラッグ
		ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.0f);

		// カメラ角度に反映
		phi += delta.x * rotationSpeed;
		theta += delta.y * rotationSpeed;

		if (phi > pi)
		{
			phi = phi - 2.0f * pi;
		} else if (phi < -pi)
		{
			phi = phi + 2.0f * pi;
		}

		// ピッチを上下90°未満にクランプ
		const float limit = 0.4999f * pi; // 約85°
		theta = std::clamp(theta, -limit, limit);

		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);

		ImGui::Text("Delta x : %3.6f, y : %3.6f", delta.x, delta.y);

	}

	LONG wheel = input_->GetMouseMove().z; // 前フレームとの差分
	radius -= wheel * 0.005f;              // 感度は 0.1f などで調整
	radius = max(1.0f, min(radius, 100.0f)); // クランプして近づきすぎ防止

	camera.rotate.x = phi;
	camera.rotate.y = theta;

	camera.translate.x = -radius * std::cos(theta) * std::sin(phi);
	camera.translate.y = radius * std::sin(theta);
	camera.translate.z = -radius * std::cos(theta) * std::cos(phi);

	/*Matrix4x4 rotMat = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, camera.rotate, { 0.0f, 0.0f, 0.0f });

	Vector3 rotatedMove =
	{
		move.x * rotMat.m[0][0] + move.y * rotMat.m[1][0] + move.z * rotMat.m[2][0],
		move.x * rotMat.m[0][1] + move.y * rotMat.m[1][1] + move.z * rotMat.m[2][1],
		move.x * rotMat.m[0][2] + move.y * rotMat.m[1][2] + move.z * rotMat.m[2][2]
	};

	debugCamera_.translate.x += rotatedMove.x;
	debugCamera_.translate.y += rotatedMove.y;
	debugCamera_.translate.z += rotatedMove.z;*/

	/*camera.translate.x = debugCamera_.translate.x + originCamera.translate.x;
	camera.translate.y = debugCamera_.translate.y + originCamera.translate.y;
	camera.translate.z = debugCamera_.translate.z + originCamera.translate.z;*/

	/*worldMatrix_ = MakeAffineMatrix(originCamera.scale, camera.rotate, camera.translate);*/

	viewMatrix_ = MakeLookAtMatrix(camera.translate, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	ImGui::End();

    ImGui::Begin("DebugCamera Info");

    ImGui::InputFloat3("translate", &camera.translate.x, "%.3f", ImGuiInputTextFlags_ReadOnly);

    ImGui::End();

#endif
}


