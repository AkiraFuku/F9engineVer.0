#pragma once
#include "MathFunction.h"
#include <vector>
#include "Vector3.h"

class RailPath {
public:
    RailPath() = default;
    ~RailPath() = default;
    void Initialize() {
        points_.clear();
    }


    // レールの点を追加
    void AddPoint(const Vector3& point) { points_.push_back(point); }

    // 全体の進捗 (0.0 ～ 区間数) から座標を取得
    Vector3 GetPosition(float globalT)const ;

    // 次の点への方向ベクトルを取得（回転制御用）
    Vector3 GetDirection(float globalT) const {
        size_t index = static_cast<size_t>(globalT);
        if (index >= points_.size() - 1) index = points_.size() - 2;
        
        return Normalize(points_[index + 1] - points_[index]);
    }

    // 最大の globalT (区間数) を取得
    float GetMaxT() const { return static_cast<float>(points_.size() - 1); }

    void DebugDraw();

private:
    
    std::vector<Vector3> points_;
};