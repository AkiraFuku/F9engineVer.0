#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "PbdSolver.h"

class PbdRope {
public:
    PbdRope() = default;
    ~PbdRope() = default;

    /// <summary>
    /// 初期化（1次元の質点列）
    /// </summary>
    void Initialize(const Vector3& startPos, const Vector3& endPos, int numPoints,
                    float k = 0.2f, float dt = 1.0f / 60.0f, float kDamping = 0.05f,
                    const Vector3& gravity = { 0.0f, -9.8f, 0.0f });

    /// <summary>
    /// 物理ステップ更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画（PrimitiveDrawer で各質点間をライン描画）
    /// </summary>
    void Draw();

    // --- セッター ---
    void SetColor(const Vector4& color) { color_ = color; }
    void SetStartPos(const Vector3& pos) { solver_.SetStartPos(pos); }
    void SetEndPos(const Vector3& pos) { solver_.SetEndPos(pos); }
    void SetK(float k) { solver_.SetK(k); }
    void SetDt(float dt) { solver_.SetDt(dt); }
    void SetMass(float mass) { solver_.SetMass(mass); }
    void SetGravity(const Vector3& g) { solver_.SetGravity(g); }
    void SetFixed(int index, bool isFixed) { solver_.SetFixed(index, 0, isFixed); }

    // --- ゲッター ---
    int GetNumPoints() const { return solver_.GetWidth(); }
    const PbdPoint& GetPoint(int index) const { return solver_.GetPoint(index, 0); }
    PbdPoint& GetPoint(int index) { return solver_.GetPoint(index, 0); }
    PbdSolver& GetSolver() { return solver_; }
    const PbdSolver& GetSolver() const { return solver_; }

private:
    PbdSolver solver_;
    Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
};
