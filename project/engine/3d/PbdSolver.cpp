#include "PbdSolver.h"
#include <cmath>

void PbdSolver::InitializeGrid(const Vector3& startPos, const Vector3& endPos, int width, int height, float k, float dt, float kDamping, const Vector3& gravity) {
    points_.clear();
    constraints_.clear();
    bendConstraints_.clear();

    startPos_ = startPos;
    endPos_   = endPos;
    width_    = width;
    height_   = height;
    springStiffness_ = k;
    dt_       = dt;
    kDamping_ = kDamping;
    gravity_  = gravity;

    if (width_ < 2 || height_ < 2) {
        return;
    }

    points_.resize(width_);
    for (int i = 0; i < width_; ++i) {
        points_[i].resize(height_);
    }

    // 格子状に質点を配置
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            float tx = static_cast<float>(i) / (width_ - 1);
            float ty = static_cast<float>(j) / (height_ - 1);

            points_[i][j].position.x = (1.0f - tx) * startPos_.x + tx * endPos_.x;
            points_[i][j].position.y = (1.0f - ty) * startPos_.y + ty * endPos_.y;
            points_[i][j].position.z = (1.0f - tx) * startPos_.z + tx * endPos_.z;

            points_[i][j].estimationPosition = points_[i][j].position;
            points_[i][j].velocity = { 0.0f, 0.0f, 0.0f };
            points_[i][j].mass = 1.0f;
            points_[i][j].isFixed = false;
        }
    }

    // デフォルト固定点: 上辺の左右2点
    points_[0][0].isFixed = true;
    points_[width_ - 1][0].isFixed = true;

    // 横方向の距離制約
    for (int i = 0; i < width_ - 1; ++i) {
        for (int j = 0; j < height_; ++j) {
            float rest = Length(points_[i][j].position - points_[i + 1][j].position);
            constraints_.push_back({ { i, j }, { i + 1, j }, rest });
        }
    }

    // 縦方向の距離制約
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_ - 1; ++j) {
            float rest = Length(points_[i][j].position - points_[i][j + 1].position);
            constraints_.push_back({ { i, j }, { i, j + 1 }, rest });
        }
    }

    // 横方向の曲げ制約（2つ離れた点）
    for (int i = 0; i < width_ - 2; ++i) {
        for (int j = 0; j < height_; ++j) {
            float rest = Length(points_[i][j].position - points_[i + 2][j].position);
            bendConstraints_.push_back({ { i, j }, { i + 2, j }, rest });
        }
    }

    // 縦方向の曲げ制約（2つ離れた点）
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_ - 2; ++j) {
            float rest = Length(points_[i][j].position - points_[i][j + 2].position);
            bendConstraints_.push_back({ { i, j }, { i, j + 2 }, rest });
        }
    }
}

void PbdSolver::InitializeRope(const Vector3& startPos, const Vector3& endPos, int numPoints, float k, float dt, float kDamping, const Vector3& gravity) {
    points_.clear();
    constraints_.clear();
    bendConstraints_.clear();

    startPos_ = startPos;
    endPos_   = endPos;
    width_    = numPoints;
    height_   = 1;
    springStiffness_ = k;
    dt_       = dt;
    kDamping_ = kDamping;
    gravity_  = gravity;

    if (width_ < 2) {
        return;
    }

    points_.resize(width_);
    for (int i = 0; i < width_; ++i) {
        points_[i].resize(1);
        float t = static_cast<float>(i) / (width_ - 1);
        points_[i][0].position.x = (1.0f - t) * startPos_.x + t * endPos_.x;
        points_[i][0].position.y = (1.0f - t) * startPos_.y + t * endPos_.y;
        points_[i][0].position.z = (1.0f - t) * startPos_.z + t * endPos_.z;

        points_[i][0].estimationPosition = points_[i][0].position;
        points_[i][0].velocity = { 0.0f, 0.0f, 0.0f };
        points_[i][0].mass = 1.0f;
        points_[i][0].isFixed = false;
    }

    // 糸の始点を固定
    points_[0][0].isFixed = true;

    // 1次元の隣接制約
    for (int i = 0; i < width_ - 1; ++i) {
        float rest = Length(points_[i][0].position - points_[i + 1][0].position);
        constraints_.push_back({ { i, 0 }, { i + 1, 0 }, rest });
    }

    // 1次元の曲げ制約
    for (int i = 0; i < width_ - 2; ++i) {
        float rest = Length(points_[i][0].position - points_[i + 2][0].position);
        bendConstraints_.push_back({ { i, 0 }, { i + 2, 0 }, rest });
    }
}

void PbdSolver::SetMass(float mass) {
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            points_[i][j].mass = mass;
        }
    }
}

void PbdSolver::SetFixed(int x, int y, bool isFixed) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        points_[x][y].isFixed = isFixed;
    }
}

void PbdSolver::Update() {
    if (width_ == 0 || height_ == 0) return;

    // 1. 外力（重力）の加算
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) {
                points_[i][j].velocity = { 0.0f, 0.0f, 0.0f };
                continue;
            }
            points_[i][j].velocity += gravity_ * dt_;
        }
    }

    // 2. 推定位置の計算
    std::vector<std::vector<Vector3>> oldPosition(width_, std::vector<Vector3>(height_));
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            oldPosition[i][j] = points_[i][j].position;
            if (points_[i][j].isFixed) {
                continue;
            }
            points_[i][j].estimationPosition = points_[i][j].position + points_[i][j].velocity * dt_;
            points_[i][j].position = points_[i][j].estimationPosition;
        }
    }

    // 3. バネ力による速度補正
    for (const auto& c : constraints_) {
        const auto& p1 = points_[c.prev.x][c.prev.y];
        const auto& p2 = points_[c.next.x][c.next.y];
        if (p1.isFixed && p2.isFixed) continue;

        float w1 = 1.0f / p1.mass;
        float w2 = 1.0f / p2.mass;
        float diff = Length(p1.position - p2.position);
        if (diff > 0.0001f && (w1 + w2) > 0.0f) {
            Vector3 dir = Normalize(p1.position - p2.position);
            Vector3 dp1 = (-springStiffness_ * w1 / (w1 + w2) * (diff - c.restLength)) * dir;
            Vector3 dp2 = (springStiffness_ * w2 / (w1 + w2) * (diff - c.restLength)) * dir;

            if (!p1.isFixed) points_[c.prev.x][c.prev.y].velocity += dp1 / dt_;
            if (!p2.isFixed) points_[c.next.x][c.next.y].velocity += dp2 / dt_;
        }
    }

    // 4. 制約の反復解決 (Solver Iterations)
    for (int iter = 0; iter < solverIterations_; ++iter) {
        // 距離制約
        for (const auto& c : constraints_) {
            auto& p1 = points_[c.prev.x][c.prev.y];
            auto& p2 = points_[c.next.x][c.next.y];

            Vector3& x1 = p1.estimationPosition;
            Vector3& x2 = p2.estimationPosition;

            Vector3 delta = x2 - x1;
            float dist = Length(delta);
            if (dist <= 0.0001f) continue;

            Vector3 correction = ((c.restLength - dist) / dist) * delta;
            float w1 = p1.isFixed ? 0.0f : (1.0f / p1.mass);
            float w2 = p2.isFixed ? 0.0f : (1.0f / p2.mass);
            float wsum = w1 + w2;
            if (wsum <= 0.0f) continue;

            if (!p1.isFixed) x1 -= (w1 / wsum) * correction;
            if (!p2.isFixed) x2 += (w2 / wsum) * correction;
        }

        // 曲げ制約
        for (const auto& bc : bendConstraints_) {
            auto& p1 = points_[bc.prev.x][bc.prev.y];
            auto& p2 = points_[bc.next.x][bc.next.y];

            Vector3& x1 = p1.estimationPosition;
            Vector3& x2 = p2.estimationPosition;

            Vector3 delta = x2 - x1;
            float dist = Length(delta);
            if (dist <= 0.0001f) continue;

            Vector3 correction = ((dist - bc.restLength) / dist * 0.5f) * delta;
            if (!p1.isFixed) x1 += correction;
            if (!p2.isFixed) x2 -= correction;
        }
    }

    // 5. 制約解決後の位置更新
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            points_[i][j].position = points_[i][j].estimationPosition;
        }
    }

    // 6. 速度の再計算
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) continue;
            points_[i][j].velocity = (points_[i][j].position - oldPosition[i][j]) / dt_;
        }
    }

    // 7. ダンピング
    VelocityDamping();

    // 8. 固定点の状態リセット
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) {
                points_[i][j].velocity = { 0.0f, 0.0f, 0.0f };
            }
        }
    }

    // 始点・終点が固定されている場合の同期
    if (points_[0][0].isFixed) {
        points_[0][0].position = startPos_;
        points_[0][0].estimationPosition = startPos_;
    }
    if (width_ > 0 && height_ > 0 && points_[width_ - 1][height_ - 1].isFixed) {
        points_[width_ - 1][height_ - 1].position = endPos_;
        points_[width_ - 1][height_ - 1].estimationPosition = endPos_;
    }
}

void PbdSolver::VelocityDamping() {
    Vector3 xcm = { 0.0f, 0.0f, 0.0f };
    Vector3 vcm = { 0.0f, 0.0f, 0.0f };
    float totalMass = 0.0f;

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) continue;
            xcm += points_[i][j].mass * points_[i][j].position;
            vcm += points_[i][j].mass * points_[i][j].velocity;
            totalMass += points_[i][j].mass;
        }
    }

    if (totalMass <= 0.0001f) return;

    xcm = xcm / totalMass;
    vcm = vcm / totalMass;

    Vector3 l = { 0.0f, 0.0f, 0.0f };
    float inertia = 0.0f;
    std::vector<std::vector<Vector3>> rs(width_, std::vector<Vector3>(height_));

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) continue;
            Vector3 r = points_[i][j].position - xcm;
            rs[i][j] = r;
            l += Cross(r, points_[i][j].mass * points_[i][j].velocity);
            inertia += Dot(r, r) * points_[i][j].mass;
        }
    }

    if (inertia <= 0.0001f) return;

    Vector3 omega = (1.0f / inertia) * l;

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            if (points_[i][j].isFixed) continue;
            Vector3 deltaV = (vcm + Cross(omega, rs[i][j])) - points_[i][j].velocity;
            points_[i][j].velocity += kDamping_ * deltaV;
        }
    }
}
