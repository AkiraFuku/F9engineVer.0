#include "BossBodyPart.h"

void BossBodyPart::OnCollision(GameObject* other)
{
        // 胴体は CollisionObject（めり込み不可）なので CollisionManager が押し出し処理等を行う
        // 必要に応じて移動制限などの処理を記述
    }