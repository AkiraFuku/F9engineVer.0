#include "RailPath.h"
#include "PrimitiveDrawer.h"
void RailPath::Update()
{

/*    // ベジェの制御点は、点の位置からのオフセットで指定されることが多いので、ここで絶対座標に変換しておく
    for (size_t i = 0; i < points_.size(); i++) {
        if (points_[i].type == InterpolationType::Bezier) {
            points_[i].controlOut = points_[i].position + points_[i].controlOut;
            points_[i].controlIn = points_[i].position + points_[i].controlIn;
        }
    }*/


}
/*Vector3 RailPath::GetPositionByCR(float globalT)const {
    if (points_.size() < 2) return points_.empty() ? Vector3{ 0,0,0 } : points_[0].position;

    size_t index = static_cast<size_t>(globalT);
    float localT = globalT - static_cast<float>(index);

    if (index >= points_.size() - 1) return points_.back().position;

    if (!points_[index].isCurve) {
        return Lerp(points_[index].position, points_[index + 1].position, localT);
    }

    // 4つの制御点の取得（外挿処理）
    Vector3 p0, p1, p2, p3;

    p1 = points_[index].position;
    p2 = points_[index + 1].position;

    // 前の点
    if (index > 0) {
        p0 = points_[index - 1].position;
    } else {
        p0 = p1 - (p2 - p1); // 始点より前の点を予測
    }

    // 後の点
    if (index + 2 < points_.size()) {
        p3 = points_[index + 2].position;
    } else {
        p3 = p2 + (p2 - p1); // 終点より先の点を予測
    }



    if (points_[index].isCurve) {
        //
        //return CatmullRom(points_[i0].position, points_[i1].position, points_[i2].position, points_[i3].position, localT);
        return CatmullRom(p0, p1, p2, p3, localT);
    } else {
        //return Lerp(points_[i1].position, points_[i2].position, localT); // 直線補間

        return Lerp(p1, p2, localT); // 直線補間
    }
}

Vector3 RailPath::GetPositionByBezier(float globalT) const {
    if (bezierPoints_.size() < 2) return bezierPoints_.empty() ? Vector3{0,0,0} : bezierPoints_[0].anchor;

    size_t index = static_cast<size_t>(globalT);
    float localT = globalT - static_cast<float>(index);

    if (index >= bezierPoints_.size() - 1) return bezierPoints_.back().anchor;

    // 現在の点と次の点
    const auto& pStart = bezierPoints_[index];
    const auto& pEnd = bezierPoints_[index + 1];

    // 4つの制御点を使ってベジェ計算
    // P0: 開始アンカー, P1: 開始点のOut制御点, P2: 終了点のIn制御点, P3: 終了アンカー
    return Bezier(pStart.anchor, pStart.controlOut, pEnd.controlIn, pEnd.anchor, localT);
}

void RailPath::DebugDraw()
{
    DebugDrawCR();
    DebugDrawBezier();

}

void RailPath::DebugDrawCR()
{
    float maxT = GetMaxTByCR();
    float step = 0.1f; // 分割精度
    for (float t = 0; t < maxT; t += step) {
        float nextT = min(t + step, maxT);
        PrimitiveDrawer::GetInstance()->DrawLine(GetPositionByCR(t), GetPositionByCR(nextT), { 1, 0, 0, 1 });
    }
}

void RailPath::DebugDrawBezier()
{
    float maxT = GetMaxTByBezier();
    float step = 0.1f; // 分割精度
    for (float t = 0; t < maxT; t += step) {
        float nextT = min(t + step, maxT);
        PrimitiveDrawer::GetInstance()->DrawLine(GetPositionByBezier(t), GetPositionByBezier(nextT), { 0, 0, 1, 1 });
    }
}*/
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
        // ※PrimitiveDrawerにDrawBoxなどがある想定
        PrimitiveDrawer::GetInstance()->DrawSphere({ p.position, 1.0f }, { 1, 1, 1, 1 }); // アンカーポイントを白い球で描画

        // ベジェハンドルの描画
        if (p.type == InterpolationType::Bezier) {
            // 出ていくハンドル (controlOut)
            PrimitiveDrawer::GetInstance()->DrawLine(p.position, p.controlOut, { 0, 0.8f, 1, 1 });
            PrimitiveDrawer::GetInstance()->DrawLine(p.controlOut, pnext.position, { 0, 0.8f, 1, 1 });

            PrimitiveDrawer::GetInstance()->DrawSphere({ p.controlOut, 1.0f }, { 0, 0.8f, 1, 1 }); // controlOutをシアンの球で描画
            // 入ってくるハンドル (controlIn)
            // ※ 次の点がある場合、次の点の controlIn と繋がっている
            if (i > 0) {
                 PrimitiveDrawer::GetInstance()->DrawLine(p.position, p.controlIn, { 0, 0.5f, 1, 1 });
                 PrimitiveDrawer::GetInstance()->DrawLine(p.controlIn, pnext.position, { 0, 0.5f, 1, 1 });

                 PrimitiveDrawer::GetInstance()->DrawSphere({ p.controlIn, 1.0f }, { 0, 0.5f, 1, 1 }); // controlInを薄いシアンの球
            }
        }
    }



}

Vector3 RailPath::GetPositionByDistance(float distance)
{
   // 1. どのセグメントにいるか探す (二分探索などが効率的)
    // 2. そのセグメント内での進捗率 (0.0~1.0) を再計算する
    // float localT = (distance - 前の累積距離) / セグメントの長さ;
    // 3. その localT を使って Catmull-Rom 等で座標計算
}

void RailPath::CalculatePathLength() {
    totalLength_ = 0.0f;
    pathData_.clear();
    
    for (size_t i = 0; i < points_.size() - 1; ++i) {
        // 本来は曲線を細分化して長さを合計するのが理想
        float len = Length(Subtract(points_[i+1].position, points_[i].position));
        totalLength_ += len;
        pathData_.push_back({totalLength_, len});
    }
}