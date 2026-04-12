#pragma once
#include "RailPath.h"
#include "Vector3.h"
class RailMover {
public:
     RailMover() = default;
    ~RailMover() = default;

    void SetPath(const RailPath* path) {
        path_ = path;
    }

    // 外部から進捗のポインタを渡せるようにする
    void BindProgress(float* externalProgress) {
        pProgress_ = externalProgress;
    }

    void Advance(float speed) {
        if (pProgress_) {
            *pProgress_ += speed; // 共有されている進捗が更新される
        } else {
            localProgress_ += speed; // 自分だけの進捗
        }
    }

    float GetProgress() const {
        return pProgress_ ? *pProgress_ : localProgress_;
    }

    // 現在の座標を取得
    Vector3 GetCurrentPosition() const {
        return path_ ? path_->GetPosition(localProgress_) : Vector3{ 0,0,0 };
    }

    // 現在の進行方向（向き）を取得
    Vector3 GetCurrentDirection() const {
        return path_ ? path_->GetDirection(localProgress_) : Vector3{ 0,0,1 };
    }

    /*    //   float GetProgress() const { return progress_; }
        void SetProgress(float p) {
            progress_ = p;
        }*/

private:
   

    const RailPath* path_ = nullptr;// 進捗の管理方法を柔軟にするため、ローカルと共有の両方を用意
    float localProgress_ = 0.0f;// 個別の進捗。これが共通でないなら、こっちを使う
    float* pProgress_ = nullptr; // これが共通なら、全員同期する
};