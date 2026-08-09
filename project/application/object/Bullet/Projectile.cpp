#include "Projectile.h"
#include "RailMover.h"
#include "ModelManager.h"
#include "Collider.h"
Projectile::Projectile() {
    railMover_ = std::make_unique<RailMover>();
    object_ = std::make_unique<Object3d>();
   
}
CollisionCategory Projectile::GetCategory() const {
    switch (owner_) {
    case ProjectileOwner::Player:
        return CollisionCategory::PlayerProjectile;
    case ProjectileOwner::Enemy:
        return CollisionCategory::EnemyProjectile;
    default:
        return CollisionCategory::PlayerProjectile; // デフォルトはプレイヤー弾とする
    }
}
Projectile::~Projectile() = default;

void Projectile::Initialize(const RailPath* path, const ProjectileSpawnParam& param, ProjectileOwner owner) {
    owner_ = owner; // 持ち主を保存
    speed_ = param.speed * param.direction.x;

    // 2. 高度方向の速度（Y方向の入力に基づき設定）
    // param.direction.y は入力の上下 (-1.0 ~ 1.0)
    velocityY_ = param.speed * param.direction.y;


    // モデルの初期化（例としてSphereを使用）
    object_->Initialize();
    object_->SetModel("playerCursor.obj"); // 必要に応じて専用モデルへ
    object_->SetScale({ 0.5f, 0.5f, 0.5f });
    //モデルの向きを進行方向に合わせるための回転を設定
    Vector3 dir3D = { param.direction.x, 0.0f, param.direction.y };
    object_->SetRotate(dir3D);


    // レール設定
    railMover_->SetPath(path);
    railMover_->SetProgress(param.position.x);
    worldY_ = param.position.y;

    collider_ = std::make_unique<Collider>();
    collider_->initialize(this, radius_);

}

void Projectile::Update() {
    if (isDead_) return;

    // レール上の位置を更新
    railMover_->Advance(speed_);

    // 【追加】高度を更新
    worldY_ += velocityY_;

    // 座標の合成
    Vector3 railPos = railMover_->GetCurrentPosition();
    Vector3 finalPos = { railPos.x, worldY_, railPos.z }; // 更新された worldY_ を使う
    object_->SetTranslate(finalPos);

    object_->Update();
    collider_->Update();
    if (--lifeTimer_ <= 0) {
        isDead_ = true;
    }
}
void Projectile::Draw() {
    if (isDead_) return;
    object_->Draw();
}

Vector3 Projectile::GetWorldPosition() const {
    return railMover_->GetCurrentPosition();
}

void Projectile::OnCollision( GameObject* other) {
    other; // 使わない場合は警告回避のために記述
    isDead_ = true; // 何かに当たったら消える
}