#pragma once
#include "RailPath.h"
#include "Vector3.h"
#include <algorithm>
class RailMover {
public:
     
    bool isRailSet() const {
        return path_ != nullptr;
    }
    void SetPath(const RailPath* path) {
        path_ = path;
    }

    // 外部から進捗のポインタを渡せるようにする
    void BindProgress(float* externalProgress) {
        pProgress_ = externalProgress;
    }

    void Advance(float speed) {
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
        }
    }

    float GetProgress() const {
        return pProgress_ ? *pProgress_ : localProgress_;
    }

    // 現在の座標を取得
    Vector3 GetCurrentPosition() const {
        if (!path_) return {0,0,0};
    return path_->GetPosition(GetProgress()); // localProgress_ ではなく GetProgress() を使う
    }

    // 現在の進行方向（向き）を取得
    Vector3 GetCurrentDirection() const {
        return path_ ? path_->GetDirection(GetProgress()) : Vector3{ 0,0,1 };
    }
private:


    const RailPath* path_ = nullptr;// 進捗の管理方法を柔軟にするため、ローカルと共有の両方を用意
    float localProgress_ = 0.0f;// 個別の進捗。これが共通でないなら、こっちを使う
    float* pProgress_ = nullptr; // これが共通なら、全員同期する
};