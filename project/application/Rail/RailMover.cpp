#include "RailMover.h"

void RailMover::Advance(float speed)
{
    // 1. 現在の「距離」を保持する変数を追加するか、tから現在の距離を逆算する
    float currentDistance = GetCurrentDistance(); // tから現在の距離をテーブルで引く
    currentDistance += speed;

    // 2. 新しい距離に対応する globalT を取得
    float nextT = path_->GetTFromDistance(currentDistance);

    // 3. 進捗を更新
    if (pProgress_) {
        *pProgress_ = nextT;
    } else {
        localProgress_ = nextT;
    }
    /*
        // 1. 現在の進捗を取得して加算
        float currentProgress = GetProgress();
        currentProgress += speed;

        // 2. 範囲を制限
        float maxT = 1.0f;
        if (path_) {
            maxT = path_->GetMaxT();
        }
        currentProgress = std::clamp(currentProgress, 0.0f, maxT);

        // 3. 【修正】クランプ済みの「最終的な値」を直接代入する
        if (pProgress_) {
            *pProgress_ = currentProgress; // += speed ではなく = currentProgress
        } else {
            localProgress_ = currentProgress; // += speed ではなく = currentProgress
        }*/
    }

float RailMover::GetCurrentDistance() const{
    if (!path_) return 0.0f;
    // RailPathに新しく作った関数を使って、現在のtから距離を取得
    return path_->GetDistanceFromT(GetProgress());
}