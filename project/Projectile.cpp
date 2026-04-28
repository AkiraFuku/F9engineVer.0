#include "Projectile.h"
#include "RailMover.h"
#include "ModelManager.h"

Projectile::Projectile() {
    railMover_ = std::make_unique<RailMover>();
    object_ = std::make_unique<Object3d>();
}

Projectile::~Projectile() = default;

void Projectile::Initialize(const RailPath* path, Vector2 start, float speed) {
    speed_ = speed;
    
    // モデルの初期化（例としてSphereを使用）
    object_->Initialize();
    object_->SetModel("Sphere"); // 必要に応じて専用モデルへ
    object_->SetScale({0.5f, 0.5f, 0.5f});

    // レール設定
    railMover_->SetPath(path);
    railMover_->SetProgress(start.x);
    worldY_ = start.y;
}

void Projectile::Update() {
    if (isDead_) return;

    railMover_->Advance(speed_);

    // 座標の更新
    Vector3 railPos = railMover_->GetCurrentPosition();
    // レールの XZ に、保存しておいた高度 Y を合成する
    Vector3 finalPos = { railPos.x, worldY_, railPos.z };
    object_->SetTranslate(finalPos);
    
    // ... 回転処理などはそのまま ...
    object_->Update();

    if (--lifeTimer_ <= 0) {
        isDead_ = true;
    }
}
void Projectile::Draw() {
    if (isDead_) return;
    object_->Draw();
}

Vector3 Projectile::GetPosition() const {
    return railMover_->GetCurrentPosition();
}

void Projectile::OnCollision() {
    isDead_ = true; // 何かに当たったら消える
}