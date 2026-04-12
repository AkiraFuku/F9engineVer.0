#include "RailPath.h"
#include "PrimitiveDrawer.h"
Vector3 RailPath::GetPosition(float globalT)const {
    if (points_.size() < 2) return points_.empty() ? Vector3{ 0,0,0 } : points_[0];

    // どの区間にいるか
    size_t index = static_cast<size_t>(globalT);
    float localT = globalT - static_cast<float>(index);

    // 終点を超えないようガード
    if (index >= points_.size() - 1) return points_.back();

    return Lerp(points_[index], points_[index + 1], localT);
}
void RailPath::DebugDraw()
{

    for (size_t i = 0; i < points_.size() - 1; ++i) {
        PrimitiveDrawer::GetInstance()->DrawLine(points_[i], points_[i + 1], { 1, 0, 0, 1 });
    }
}
