#pragma once
#include "MathFunction.h"
#include <vector>
#include "Vector3.h"

class RailPath {
public:
    enum class InterpolationType {
        Linear,     // 直線
        CatmullRom, // キャットムル・ロム
        Bezier      // 三次ベジェ
    };

    struct RailPoint {
        Vector3 position;
        InterpolationType type = InterpolationType::Linear;

        // ベジェ用：この点から「出ていく」制御点と、次の点へ「入る」制御点
        // Catmull-Romの場合は自動計算されるため使いません
        Vector3 controlOut;
        Vector3 controlIn;
    };

    void Initialize() {
        points_.clear();
    }
    void AddPoint(const Vector3& pos) {
        points_.push_back({ pos,  InterpolationType::Linear, {0,0,0}, {0,0,0} });
    }
    void AddPointCR(const Vector3& pos) {
        // control点は使わないのでゼロ初期化
        points_.push_back({ pos,  InterpolationType::CatmullRom, {0,0,0}, {0,0,0} });
    }


    Vector3 GetPointPos(size_t index) const {

        if (index >= points_.size()) return { 0,0,0 };
        return points_[index].position;
    }

    Vector3 GetControlPointOut(size_t index) const {
        if (index >= points_.size()) return { 0,0,0 };
        return points_[index].controlOut;
    }
    Vector3 GetControlPointIn(size_t index) const {
        if (index >= points_.size()) return { 0,0,0 };
        return points_[index].controlIn;
    }

    void SetPointPos(size_t index, const Vector3& pos)
    {
        if (index < points_.size()) {
            points_[index].position = pos;
        }
    }

    void SetControlPointOut(size_t index, const Vector3& control) {
        if (index < points_.size()) {
            points_[index].controlOut = control;
        }
    }
    void SetControlPointIn(size_t index, const Vector3& control) {
        if (index < points_.size()) {
            points_[index].controlIn = control;
        }
    }
    void Update();

    // ベジェ曲線として点を追加
    void AddBezierPoint(const Vector3& pos, const Vector3& cIn, const Vector3& cOut) {
        points_.push_back({ pos, InterpolationType::Bezier, cIn, cOut });
    }

    Vector3 GetPosition(float globalT) const {
        if (points_.size() < 2) return points_.empty() ? Vector3{ 0,0,0 } : points_[0].position;

        size_t index = static_cast<size_t>(globalT);
        float t = globalT - static_cast<float>(index);

        if (index >= points_.size() - 1) return points_.back().position;

        const auto& p1 = points_[index];
        const auto& p2 = points_[index + 1];

        switch (p1.type) {
        case InterpolationType::Linear:
            return Lerp(p1.position, p2.position, t);

        case InterpolationType::Bezier:
            // p1から出るハンドルと、p2へ入るハンドルを使用
            return Bezier(p1.position, p1.controlOut, p2.controlIn, p2.position, t);

        case InterpolationType::CatmullRom: {
            // 前後の点を取得（外挿処理含む）
            Vector3 p0 = (index > 0) ? points_[index - 1].position : p1.position - (p2.position - p1.position);
            Vector3 p3 = (index + 2 < points_.size()) ? points_[index + 2].position : p2.position + (p2.position - p1.position);
            return CatmullRom(p0, p1.position, p2.position, p3, t);
        }
        }
        return p1.position;
    }
    // 次の点への方向ベクトルを取得（回転制御用）
    Vector3 GetDirection(float globalT) const;
    float GetMaxT() const {
        return points_.empty() ? 0.0f : static_cast<float>(points_.size() - 1);
    }

    void DebugDraw();

private:
    std::vector<RailPoint> points_;
};