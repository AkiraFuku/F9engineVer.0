#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include <memory>
class Object3d;
struct Move {
	Vector2 max;
	Vector2 min;
};
class Camera;
class Player;
class RailMover;
class RailPath;
class CameraController {
public:
	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};

    CameraController() ;
    ~CameraController();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; }
	void SetMoveArea(const Rect area) { moveArea_ = area; }

    void SetRailPath(const RailPath* path) ;
    void SetRailProgress(float progress);
    

	void Reset();
	void RequestShake(float duration, float power);
	void SetClearOffset() { 
        // プレイヤーに近づける（Zを近づけ、少し見上げるような座標にする例）
        targetOffsetGoal_ = {0.0f, 0.0f, -6.0f}; 
    }
	void TriggerClearFocus() { isClearPhase_ = true; }
private:
	// カメラ
	Camera* camera_ = nullptr;
	Player* target_ = nullptr;
    Object3d* target=nullptr;
    std::unique_ptr<RailMover> railMover_;
	Rect moveArea_ = {0.0f, 100.0f, 0.0f, 100.0f};
	Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
	Vector3 targetOffsetGoal_ = {0.0f, 0.0f, -15.0f};
	bool isClearPhase_ = false;
    Vector3 clearOffset_ = {0.0f, 1.0f, -5.0f};
	Vector3 desetination_;
	static inline const float kInterpolationRate = 0.1f;                 //
	static inline const float kVelocityBias = 30.0f;                     //
	static inline const Rect targetMargin_ = {-9.0f, 9.0f, -5.0f, 5.0f}; // ターゲットのマージン

	

    void RotateCamera();
    void RailCamera();
    // シェイク用変数
    float shakeTimer_ = 0.0f;
	float shakePower_ = 0.0f;
    Vector3 shakeOffset_ = { 0.0f, 0.0f, 0.0f };
    // シェイクの更新処理 タイマーとオフセットを更新する
    void UpdateShake();
public:
    void ShakeCamera(float duration=0.1f, float power=1.0f) {
        shakeTimer_ = duration;
        shakePower_ = power;
    }

};


