#include "BossWeakPart.h"
#include "MiniBoss.h"

void BossWeakPart::OnCollision(GameObject* other)
{
        if (!other) return;

        // プレイヤーの攻撃（弾など）が当たった場合
        if (other->GetCategory() == CollisionCategory::PlayerProjectile || 
            other->GetCategory() == CollisionCategory::Player) {
            
            // 親ボスに弱点ヒットのダメージを通知
           // ownerBoss_->TakeDamage(2); // 弱点なので大ダメージ
          //  ownerBoss_->PlayWeakHitEffect(GetWorldPosition());
        }
    }