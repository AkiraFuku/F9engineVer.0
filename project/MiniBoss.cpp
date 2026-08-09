#include "MiniBoss.h"
#include "ModelManager.h"
#include "MathFunction.h"
#include "Collider.h"
void MiniBoss::Initialize() {
    // 1. 基底の基本初期化
    Enemy::Initialize();

    // 2. モデルの生成（必要に応じて作成）
    ModelManager::GetInstance()->CreateSphereModel("BossBody");
    ModelManager::GetInstance()->CreateSphereModel("BossWeak");
    ModelManager::GetInstance()->CreateSphereModel("BossArmor");

    // --- パーツ1: 胴体 (めり込めない障害物属性) ---
    auto bodyPart = std::make_unique<BossPart>();
    bodyPart->type = BossPartType::Body;
    bodyPart->object = std::make_unique<Object3d>();
    bodyPart->object->Initialize();
    bodyPart->object->SetModel("BossBody");
    bodyPart->collider = std::make_unique<Collider>();
    bodyPart->collider->initialize(this, 3.0f);
    bodyPart->collider->SetCategory(CollisionCategory::CollisionObject); // めり込み不可
    bodyPart->localPosition = { 0.0f, 0.0f, 0.0f };
    parts_.push_back(std::move(bodyPart));

    // --- パーツ2: 弱点 (攻撃可能オブジェクト属性) ---
    auto weakPart = std::make_unique<BossPart>();
    weakPart->type = BossPartType::Weak;
    weakPart->object = std::make_unique<Object3d>();
    weakPart->object->Initialize();
    weakPart->object->SetModel("BossWeak");
    weakPart->collider = std::make_unique<Collider>();
    weakPart->collider->initialize(this, 1.2f);
    weakPart->collider->SetCategory(CollisionCategory::Attackable); // 攻撃可能
    weakPart->localPosition = { 0.0f, 2.0f, 0.0f }; // 胴体の上に配置
    parts_.push_back(std::move(weakPart));

    // --- パーツ3: 盾/装甲 (攻撃不可エネミー属性) ---
    auto armorPart = std::make_unique<BossPart>();
    armorPart->type = BossPartType::Armor;
    armorPart->object = std::make_unique<Object3d>();
    armorPart->object->Initialize();
    armorPart->object->SetModel("BossArmor");
    armorPart->collider = std::make_unique<Collider>();
    armorPart->collider->initialize(this, 1.5f);
    armorPart->collider->SetCategory(CollisionCategory::InvincibleEnemy); // 攻撃無効
    armorPart->localPosition = { 0.0f, 0.0f, 2.0f }; // 前方に配置
    parts_.push_back(std::move(armorPart));
}

void MiniBoss::Update() {
    // 位置・物理更新（親の処理を実行）
    Enemy::Update();

    Vector3 bossPos = GetWorldPosition();

    // 各パーツの位置をボスのワールド座標に合わせて追従させる
    for (auto& part : parts_) {
        Vector3 worldPos = Add(bossPos, part->localPosition);
        part->object->SetTranslate(worldPos);
        part->object->Update();
    }
}

void MiniBoss::Draw() {
    for (auto& part : parts_) {
        if (part->object) {
            part->object->Draw();
        }
    }
}

void MiniBoss::OnCollision(GameObject* other) {
    if (!other || isDamaged_ || IsDead()) return;

    // 攻撃（弾やプレイヤーの攻撃）を受けた場合の処理
    if (other->GetCategory() == CollisionCategory::PlayerProjectile || 
        other->GetCategory() == CollisionCategory::Player) {
        
        // HPを減らす処理など
        hp_--;
        PlayHitEffect();

        if (hp_ <= 0) {
            ChangeState(std::make_unique<StateEnemyDead>());
        }
    }
}