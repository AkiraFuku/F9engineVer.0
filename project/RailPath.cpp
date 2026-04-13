#include "RailPath.h"
#include "PrimitiveDrawer.h"
Vector3 RailPath::GetPosition(float globalT)const {
    if (points_.size() < 2) return points_.empty() ? Vector3{ 0,0,0 } : points_[0].position;

    // どの区間にいるか
    size_t index = static_cast<size_t>(globalT);
    float localT = globalT - static_cast<float>(index);

    // 終点を超えないようガード
    if (index >= points_.size() - 1) return points_.back().position;

    // 4つの制御点を決定 (端の処理)
    size_t i0 = (index == 0) ? 0 : index - 1;
    size_t i1 = index;
    size_t i2 = index + 1;
    size_t i3 = (index + 2 >= points_.size()) ? i2 : index + 2;

 if (points_[index].isCurve) {
    return CatmullRom(points_[i0].position, points_[i1].position, points_[i2].position, points_[i3].position, localT);
} else {
    return Lerp(points_[i1].position, points_[i2].position, localT); // 直線補間
}
}
void RailPath::DebugDraw()
{
    float maxT = GetMaxT();
    float step = 0.1f; // 分割精度
    for (float t = 0; t < maxT; t += step) {
        float nextT = min(t + step, maxT);
        PrimitiveDrawer::GetInstance()->DrawLine(GetPosition(t), GetPosition(nextT), { 1, 0, 0, 1 });
    }
}
