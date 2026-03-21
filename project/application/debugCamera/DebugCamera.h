#pragma once
#include "mathFunction.h"
#include <Windows.h>
#include "numbers"
#include <algorithm>
#include "DebugCameraFunction.h"
#include "Transform.h"

class Input;



/// <summary>
///  デバッグカメラ
/// </summary>
class DebugCamera
{
public:

	Input* input_;

	/// <summary>
	///  初期化
	/// </summary>
	void Initialize();

	/// <summary>
	///  更新
	/// </summary>
	void Update(EulerTransform camera);

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }


private:

	// カメラ
	EulerTransform debugCamera_;

	// ビュー行列
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	// 射影行列
	Matrix4x4 projectionMatrix_ = MakeIdentity4x4();

	float radius = 53.0f;    // 原点からの距離
	const float rotationSpeed = 0.005f; // ドラッグ速度の調整係数

	float theta = 0.0f;
	float phi = 0.0f;

	// マウス座標を取得
	int mouseX, mouseY;

};
