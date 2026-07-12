#include "RailMover.h"

void RailMover::Advance(float speed) {
    if (!path_) return;
    if (speed > 0) currentDir_ = MoveDirection::Forward;
    else if (speed < 0) currentDir_ = MoveDirection::Backward;
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
void RailMover::SyncWith(const RailMover* otherMover){

    if (!path_ || !otherMover || !otherMover->path_) return;

    // 1. 同期相手のレール情報と現在の距離を取得
    const RailPath* otherPath = otherMover->path_;
    float otherDist = otherMover->GetCurrentDistance();
    float otherTotalLen = otherPath->GetTotalLength();

    // 2. 相手が「全行程の何％」にいるかを算出 (0.0f ～ 1.0f)
    if (otherTotalLen <= 0.0f) return; // 0除算防止
    float progressRate = otherDist / otherTotalLen;

    // 3. 自分のレールの「同じ％」にあたる距離を算出
    float myTotalLen = path_->GetTotalLength();
    float myTargetDist = myTotalLen * progressRate;

    // 4. その距離に対応する T を取得して自分に適用
    float myNextT = path_->GetTFromDistance(myTargetDist);
    
    if (pProgress_) {
        *pProgress_ = myNextT;
    } else {
        localProgress_ = myNextT;
    }
}
float RailMover::GetCurrentDistance() const{
    if (!path_) return 0.0f;
    // RailPathに新しく作った関数を使って、現在のtから距離を取得
    return path_->GetDistanceFromT(GetProgress());
}