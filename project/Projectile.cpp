#include "Projectile.h"
#include "RailMover.h"
#include "ModelManager.h"

Projectile::Projectile() {
    railMover_ = std::make_unique<RailMover>();
    object_ = std::make_unique<Object3d>();
}

Projectile::~Projectile() = default;

void Projectile::Initialize(const RailPath* path, float startT, float speed) {
    speed_ = speed;
    
    // モデルの初期化（例としてSphereを使用）
    object_->Initialize();
    object_->SetModel("Sphere"); // 必要に応じて専用モデルへ
    object_->SetScale({0.5f, 0.5f, 0.5f});

    // レール設定
    railMover_->SetPath(path);
    railMover_->SetProgress(startT);
}

void Projectile::Update() {
    if (isDead_) return;

    // レール上を進める
    railMover_->Advance(speed_);

    // 座標と回転を更新
    object_->SetTranslate(railMover_->GetCurrentPosition());
    
    // 進行方向を向く（Playerの回転ロジックを参考）
    Vector3 dir = railMover_->GetCurrentDirection();
    float angle = atan2f(dir.x, dir.z);
    object_->SetRotate({ 0.0f, angle, 0.0f });

    object_->Update();

    // 寿命尽きたら消滅
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