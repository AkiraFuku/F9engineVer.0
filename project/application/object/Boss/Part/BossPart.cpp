#include "BossPart.h"
#include "MiniBoss.h" // ownerBoss_ を使うなら必要に応じて

Vector3 BossPart::GetWorldPosition() const {

    if (ownerBoss_) {
        return ownerBoss_->GetPosition() + localPosition_;
    }
    return localPosition_; // ownerBoss_ が nullptr の場合はローカル位置を返す

}

