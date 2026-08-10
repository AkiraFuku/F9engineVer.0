#include "BossArmorPart.h"

void BossArmorPart::OnCollision(GameObject* other)
{
    if (!other) return;

    // プレイヤーの弾が当たった場合
    if (other->GetCategory() == CollisionCategory::PlayerProjectile) {
        // ダメージは与えず、弾きエフェクトや音だけ鳴らす
   //     ownerBoss_->PlayArmorReflectEffect(GetWorldPosition());
    }
};

