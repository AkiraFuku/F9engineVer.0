#include "RailPath.h"
#include "PrimitiveDrawer.h"
void RailPath::Update()
{
    BuildDistanceTable();
}

void RailPath::SetBezierHandles(size_t index, const Vector3& offsetIn, const Vector3& offsetOut) {
    if (index < points_.size()) {
        points_[index].type = InterpolationType::Bezier;
        points_[index].controlIn = points_[index].position + offsetIn;
        points_[index].controlOut = points_[index].position + offsetOut;
    }
}

// 次の点への方向ベクトルを取得（回転制御用）
Vector3 RailPath::GetDirection(float globalT) const {
    const float delta = 0.01f; // 微小な変化量
    const float maxT = GetMaxT();

    // 現在の座標
    Vector3 p1 = GetPosition(globalT);

    // わずか先の座標（最大値を超えないようにクランプ）
    float nextT = min(globalT + delta, maxT);
    
    // もし終端に達していて差分が取れない場合は、手前の点との差分で代用
    if (globalT >= maxT) {
        Vector3 p0 = GetPosition(max(0.0f, globalT - delta));
        return Normalize(p1 - p0);
    }

    Vector3 p2 = GetPosition(nextT);

    // 2点間の差分から方向を求める
    return Normalize(p2 - p1);
}

float RailPath::GetTFromDistance(float distance) const
{
    if (distance <= 0) return 0.0f;
    if (distance >= totalLength_) return GetMaxT();

    // 二分探索などで、指定された距離に対応する t を探す
    for (size_t i = 0; i < distanceTable_.size() - 1; ++i) {
        if (distance <= distanceTable_[i+1].distance) {
            float d1 = distanceTable_[i].distance;
            float d2 = distanceTable_[i+1].distance;
            float t1 = distanceTable_[i].t;
            float t2 = distanceTable_[i+1].t;
            // 距離に基づいて線形補間
            float factor = (distance - d1) / (d2 - d1);
            return t1 + (t2 - t1) * factor;
        }
    }
    return GetMaxT();
}
void RailPath::BuildDistanceTable() {
    distanceTable_.clear();
    totalLength_ = 0.0f;
    float maxT = GetMaxT();
    float step = 0.05f;

    Vector3 prevPos = GetPosition(0.0f);
    distanceTable_.push_back({ 0.0f, 0.0f });

    for (float t = step; t <= maxT + 0.001f; t += step) {
        // maxTを超えないように clamp（浮動小数点の誤差対策）
        float currentT = (t > maxT) ? maxT : t;
        Vector3 currentPos = GetPosition(currentT);
        totalLength_ += Length(currentPos - prevPos);
        distanceTable_.push_back({ totalLength_, currentT });
        prevPos = currentPos;
    }
}
void RailPath::DebugDraw()
{

    if (points_.empty()) return;

    float maxT = GetMaxT();
    float step = 0.1f; // 分割精度

    // --- 1. レール本体の描画 ---
    for (float t = 0; t < maxT; t += step) {
        float nextT = min(t + step, maxT);
        
        // 区間の開始インデックスから補間タイプを特定（色を変えると分かりやすい）
        size_t index = static_cast<size_t>(t);
        Vector4 color = { 1, 1, 1, 1 }; // デフォルト：白

        if (points_[index].type == InterpolationType::Linear)     color = { 1, 1, 0, 1 }; // 黄
        if (points_[index].type == InterpolationType::CatmullRom) color = { 1, 0, 0, 1 }; // 赤
        if (points_[index].type == InterpolationType::Bezier)     color = { 0, 1, 0, 1 }; // 緑

        PrimitiveDrawer::GetInstance()->DrawLine(GetPosition(t), GetPosition(nextT), color);
    }

    // --- 2. 制御点（アンカー）とハンドルの描画 ---
    for (size_t i = 0; i < points_.size(); ++i) {
        const auto& p = points_[i];
        const auto& pnext = (i + 1 < points_.size()) ? points_[i + 1] : p; // 次の点（存在しない場合は同じ点）

        // アンカーポイント（実際に通る点）を立方体や球で描画
        // ※PrimitivaeDrawerにDrawBoxなどがある想定
        PrimitiveDrawer::GetInstance()->DrawSphere({ p.position, 0.5f }, { 1, 1, 1, 1 }); // アンカーポイントを白い球で描画

        // ベジェハンドルの描画
        if (p.type == InterpolationType::Bezier) {
            // 出ていくハンドル (controlOut)
            PrimitiveDrawer::GetInstance()->DrawLine(p.position, p.controlOut, { 0, 0.8f, 1, 1 });
            PrimitiveDrawer::GetInstance()->DrawLine(p.controlOut, pnext.position, { 0, 0.8f, 1, 1 });

            PrimitiveDrawer::GetInstance()->DrawSphere({ p.controlOut, 0.25f ,{ 0.0f,0.0f,0.0f,1.0f }}, { 0, 1.0f, 1, 1 }); // controlOutをシアンの球で描画
            // 入ってくるハンドル (controlIn)
            // ※ 次の点がある場合、次の点の controlIn と繋がっている
            if (i > 0) {
                 PrimitiveDrawer::GetInstance()->DrawLine(p.position, p.controlIn, { 0, 0.5f, 1, 1 });
                 PrimitiveDrawer::GetInstance()->DrawLine(p.controlIn, pnext.position, { 0, 0.5f, 1, 1 });

                 PrimitiveDrawer::GetInstance()->DrawSphere({ p.controlIn, 0.25f,{ 0.0f,0.0f,0.0f,1.0f } }, { 0, 1.0f, 1, 1 }); // controlInを薄いシアンの球
            }
        }
    }



}
float RailPath::GetDistanceFromT(float t) const {
    if (distanceTable_.empty()) return 0.0f;
    if (t <= 0) return 0.0f;
    if (t >= GetMaxT()) return totalLength_;

    // t に対応する区間をテーブルから探す
    for (size_t i = 0; i < distanceTable_.size() - 1; ++i) {
        if (t <= distanceTable_[i + 1].t) {
            float t1 = distanceTable_[i].t;
            float t2 = distanceTable_[i + 1].t;
            float d1 = distanceTable_[i].distance;
            float d2 = distanceTable_[i + 1].distance;

            // t に基づいて距離を線形補間
            float factor = (t - t1) / (t2 - t1);
            return d1 + (d2 - d1) * factor;
        }
    }
    return totalLength_;
}