#include "RailMover.h"

void RailMover::Advance(float speed) {
    if (!path_) return;

    float currentDistance = GetCurrentDistance();
    currentDistance += speed;

    float totalLen = path_->GetTotalLength();

    if (path_->IsLoop()) { // 仮に IsLoop() というゲッターがある想定
        // --- ループ時：無限周回対応 ---
        currentDistance = fmodf(currentDistance, totalLen);
        if (currentDistance < 0) currentDistance += totalLen;
    } else {
        // --- 非ループ時：始点と終点でクランプ（固定） ---
        currentDistance = std::clamp(currentDistance, 0.0f, totalLen);
    }

    float nextT = path_->GetTFromDistance(currentDistance);

    if (pProgress_) {
        *pProgress_ = nextT;
    } else {
        localProgress_ = nextT;
    }
}
float RailMover::GetCurrentDistance() const{
    if (!path_) return 0.0f;
    // RailPathに新しく作った関数を使って、現在のtから距離を取得
    return path_->GetDistanceFromT(GetProgress());
}