#pragma once
#include "MathFunction.h"
#include <vector>
#include "Vector3.h"

class RailPath {
public:
    struct PathPoint {
        Vector3 position;
        bool isCurve; // 次の点まで曲線にするか直線にするか
        PathPoint* next; // 次の点へのポインタ（曲線制御用）
        PathPoint* prev; // 前の点へのポインタ（曲線制御用）
    };

    void Initialize() {
        points_.clear();
    }


    // レールの点を追加
    void AddPoint(const Vector3& point,bool isCurve=false) {

        if (points_.empty())
        {
            points_.push_back({ point, isCurve, nullptr, nullptr });

        } else
        {
            PathPoint& lastPoint = points_.back();
            lastPoint.next = new PathPoint{ point, isCurve, nullptr, &lastPoint };
            points_.push_back(*lastPoint.next);


        }

    }

    // 全体の進捗 (0.0 ～ 区間数) から座標を取得
    Vector3 GetPosition(float globalT)const;

    // 次の点への方向ベクトルを取得（回転制御用）
    Vector3 GetDirection(float globalT) const {
        float delta = 0.01f; // 微小な変化量
        Vector3 p1 = GetPosition(globalT);
        Vector3 p2 = GetPosition(std::min(globalT + delta, GetMaxT()));

        // 2点間の差分から方向を求める
        return Normalize(p2 - p1);
    }

    // 最大の globalT (区間数) を取得
    float GetMaxT() const {
        return static_cast<float>(points_.size() - 1);
    }

    void DebugDraw();

private:

    std::vector<PathPoint> points_;
};