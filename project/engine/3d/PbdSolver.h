#pragma once

#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "MathFunction.h"

// 質点構造体
struct PbdPoint {
    Vector3 position;
    Vector3 estimationPosition;
    Vector3 velocity;
    float   mass = 1.0f;
    bool    isFixed = false;
};

// 制約構造体（距離制約・曲げ制約を統合）
struct PbdConstraint {
    int2  prev;       // 接続点1 (x=i, y=j)
    int2  next;       // 接続点2 (x=i, y=j)
    float restLength; // 静止長
};

class PbdSolver {
public:
    PbdSolver() = default;
    ~PbdSolver() = default;

    /// <summary>
    /// 2次元グリッドとして初期化
    /// </summary>
    void InitializeGrid(const Vector3& startPos, const Vector3& endPos, int width, int height, float k, float dt, float kDamping, const Vector3& gravity);

    /// <summary>
    /// 1次元の糸・ロープとして初期化
    /// </summary>
    void InitializeRope(const Vector3& startPos, const Vector3& endPos, int numPoints, float k, float dt, float kDamping, const Vector3& gravity);

    /// <summary>
    /// 物理ステップ更新
    /// </summary>
    void Update();

    /// <summary>
    /// 速度ダンピング（角運動量・重心速度を考慮した減衰）
    /// </summary>
    void VelocityDamping();

    // --- セッター ---
    void SetStartPos(const Vector3& startPos) { startPos_ = startPos; }
    void SetEndPos(const Vector3& endPos) { endPos_ = endPos; }
    void SetK(float springStiffness) { springStiffness_ = springStiffness; }
    void SetDt(float dt) { dt_ = dt; }
    void SetGravity(const Vector3& gravity) { gravity_ = gravity; }
    void SetDamping(float damping) { kDamping_ = damping; }
    void SetSolverIterations(int iterations) { solverIterations_ = iterations; }

    void SetMass(float mass);
    void SetFixed(int x, int y, bool isFixed);

    // --- ゲッター ---
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    const PbdPoint& GetPoint(int x, int y) const { return points_[x][y]; }
    PbdPoint& GetPoint(int x, int y) { return points_[x][y]; }
    const std::vector<std::vector<PbdPoint>>& GetPoints() const { return points_; }
    const std::vector<PbdConstraint>& GetDistanceConstraints() const { return constraints_; }
    const std::vector<PbdConstraint>& GetBendConstraints() const { return bendConstraints_; }

private:
    int width_  = 0; // 横の点数
    int height_ = 0; // 縦の点数

    Vector3 startPos_ = { 0.0f, 0.0f, 0.0f };
    Vector3 endPos_   = { 0.0f, 0.0f, 0.0f };

    float springStiffness_ = 0.1f; // バネの硬さ(k)
    float dt_              = 1.0f / 60.0f;
    float kDamping_        = 0.05f;
    int   solverIterations_ = 5;
    Vector3 gravity_       = { 0.0f, -9.8f, 0.0f };

    std::vector<std::vector<PbdPoint>> points_;
    std::vector<PbdConstraint> constraints_;     // 隣接距離制約
    std::vector<PbdConstraint> bendConstraints_; // 曲げ制約
};
