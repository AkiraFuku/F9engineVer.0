#include "MiniBoss.h"
#include "ModelManager.h"
#include "MathFunction.h"
#include "Collider.h"
#include "BossBodyPart.h"
#include "BossWeakPart.h"
#include "BossArmorPart.h"
#include "EnemyState.h"
#include "ImGuI.h"

MiniBoss::MiniBoss() = default;
MiniBoss::~MiniBoss() = default;

void MiniBoss::Initialize() {
   // Enemy::Initialize();

    ModelManager::GetInstance()->CreateSphereModel("BossBody");
    ModelManager::GetInstance()->CreateSphereModel("BossWeak");
    ModelManager::GetInstance()->CreateSphereModel("BossArmor");

    // 1. 胴体パーツ
    auto body = std::make_unique<BossBodyPart>(this, BossPartType::Body, 3.0f, CollisionCategory::CollisionObject, Vector3{ 0.0f, 0.0f, 0.0f });
    auto bodyObj = std::make_unique<Object3d>();
    bodyObj->Initialize();
    bodyObj->SetModel("BossBody");
    bodyObj->GetModel()->SetColor({ 0.5f, 0.5f, 0.5f, 1.0f }); // グレーに設定

    body->SetObject(std::move(bodyObj));
    parts_.push_back(std::move(body));

    // 2. 弱点パーツ（大ダメージ）
    auto weak = std::make_unique<BossWeakPart>(this, BossPartType::Weak, 1.2f, CollisionCategory::Attackable, Vector3{ 0.0f, 2.0f, 0.0f });
    auto weakObj = std::make_unique<Object3d>();
    weakObj->Initialize();
    weakObj->SetModel("BossWeak");
    weakObj->GetModel()->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 赤に設定
    weak->SetObject(std::move(weakObj));
    parts_.push_back(std::move(weak));

    // 3. 装甲パーツ（無効化）
    auto armor = std::make_unique<BossArmorPart>(this, BossPartType::Armor, 1.5f, CollisionCategory::InvincibleEnemy, Vector3{ 0.0f, 0.0f, 2.0f });
    auto armorObj = std::make_unique<Object3d>();
    armorObj->Initialize();
    armorObj->SetModel("BossArmor");
    armorObj->GetModel()->SetColor({ 0.5f, 0.5f, 0.5f, 1.0f }); // グレーに設定
    armor->SetObject(std::move(armorObj));
    parts_.push_back(std::move(armor));


}

void MiniBoss::Update() {
    //Enemy::Update();

    

    // 各パーツの位置・モデル・コライダーの更新
    for (auto& part : parts_) {
        Vector3 worldPos = part->GetWorldPosition();
        if (part->GetObject3D()) {
            part->GetObject3D()->SetTranslate(worldPos+position_);
            part->GetObject3D()->Update();
        }
        if (part->GetCollider()) {
            part->GetCollider()->Update();
        }
    }

#ifdef USE_IMGUI

    ImGui::Begin("MiniBoss Debug");
    ImGui::Text("MiniBoss HP: %d", hp_);
    //ImGui::Text("MiniBoss State: %s", GetStateName());
    //ImGui::Text("MiniBoss Behavior: %s", GetBehaviorName());
    ImGui::End();


#endif // USE_IMGUI

}

void MiniBoss::Draw()
{
    for (const auto& part : parts_) {
        if (part->GetObject3D()) {
            part->GetObject3D()->Draw();

        }
        if (part->GetCollider())
        {
            part->GetCollider()->Draw();

        }
    }
}

// ボス全体のHP減算などのインターフェースを用意
void MiniBoss::TakeDamage(int damage) {
//    if (isDamaged_ || IsDead()) return;

    hp_ -= damage;
    isDamaged_ = true;
//    hitInvincibilityTimer_ = kHitInvincibilityDuration_;

    if (hp_ <= 0) {
//        ChangeState(std::make_unique<StateEnemyDead>());
    }
}

std::vector<Collider*> MiniBoss::GetColliders() const
{
    std::vector<Collider*> colliders;
    for (const auto& part : parts_) {
        if (part->GetCollider()) {
            colliders.push_back(part->GetCollider());
        }
    }
    return colliders;
}

//void MiniBoss::OnCollision(GameObject* other)
//{
//    other;
//}
