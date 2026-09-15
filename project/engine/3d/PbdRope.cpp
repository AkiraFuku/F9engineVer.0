#include "PbdRope.h"
#include "PrimitiveDrawer.h"

void PbdRope::Initialize(const Vector3& startPos, const Vector3& endPos, int numPoints,
                         float k, float dt, float kDamping, const Vector3& gravity) {
    solver_.InitializeRope(startPos, endPos, numPoints, k, dt, kDamping, gravity);
}

void PbdRope::Update() {
    solver_.Update();
}

void PbdRope::Draw() {
    int numPoints = solver_.GetWidth();
    if (numPoints < 2) return;

    auto drawer = PrimitiveDrawer::GetInstance();
    const auto& points = solver_.GetPoints();

    // 質点間をラインで結んで描画
    for (int i = 0; i < numPoints - 1; ++i) {
        drawer->DrawLine(points[i][0].position, points[i + 1][0].position, color_);
    }
}
