#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include <functional>
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
	enum class CameraMode {
		AutoOffset, ///< レール形状から外側・高さオフセットを自動計算追従（推奨）
		RailCamera, ///< 既存のCameraRailに沿って追従
	};

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
    void SetStageRail(const RailPath* path) { stageRail_ = path; }
    void SetRailProgress(float progress);
    
    // カメラモード・オフセット設定
    void SetCameraMode(CameraMode mode) { mode_ = mode; }
    CameraMode GetCameraMode() const { return mode_; }
    void SetAutoOffsetParams(float distance, float height, bool flipSide = false) {
        autoDistance_ = distance;
        autoHeight_ = height;
        isFlipSide_ = flipSide;
    }
    void SetDrawDistance(float distance);
    float GetDrawDistance() const { return drawDistance_; }
    Camera* GetCamera() const { return camera_; }

	void Reset();
	// シェイクを要求する関数（デフォルトで二次関数的な減衰イージングを指定）
    void RequestShake(
        float duration, 
        float power, 
        std::function<float(float)> easingFunc = [](float t) { return (1.0f - t) * (1.0f - t); } // EaseOutQuad のような減衰
    );
    void ShakeCamera(float duration = 0.1f, float power = 1.0f) {
        RequestShake(duration, power);
    }
	void SetClearOffset() { 
        targetOffsetGoal_ = {0.0f, 0.0f, -6.0f}; 
    }
	void TriggerClearFocus() { isClearPhase_ = true; }
private:
	// カメラ
	Camera* camera_ = nullptr;
    const RailPath* stageRail_ = nullptr; ///< ステージレール（自動オフセット計算用）
    CameraMode mode_ = CameraMode::AutoOffset; ///< デフォルトは自動オフセット計算カメラ
    float autoDistance_ = 25.0f;          ///< レールからの水平外側オフセット距離
    float autoHeight_   = 5.0f;           ///< レールからの高さオフセット
    float lookAtHeight_ = 2.0f;           ///< 注視点の高さオフセット
    float smoothSpeed_  = 0.15f;          ///< カメラ位置の追従補間率
    bool  isFlipSide_   = false;          ///< オフセット向き反転
    float drawDistance_ = 80.0f;          ///< 描画距離範囲 (FarClip)
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
    void AutoOffsetCamera();
    // シェイク用変数
   float shakeTimer_ = 0.0f;
    float shakeDuration_ = 0.0f; // 追加: シェイク開始時のトータル時間を記録
    float shakePower_ = 0.0f;
    Vector3 shakeOffset_ = { 0.0f, 0.0f, 0.0f };

    // 追加: イージング関数を保存する変数
    std::function<float(float)> shakeEasing_ = nullptr;
    void UpdateShake();



};


