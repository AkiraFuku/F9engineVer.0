#include "Collider.h"
#include "GameObject.h"
#include "PrimitiveDrawer.h"
#include "MathFunction.h"
#include "RotateFunction.h"

void Collider::initialize(GameObject* owner, float radius)
{
    owner_ = owner;
    category_ = owner->GetCategory();
    radius_ = radius;

    Offset_ = { 0.0f, 0.0f, 0.0f };
    rotation_ = { 0.0f, 0.0f, 0.0f, 1.0f };
    sphereDefs_.clear();

    position_ = owner_->GetWorldPosition();
}

void Collider::Update()
{
    if (owner_) {
        position_ = owner_->GetWorldPosition() + Offset_;
    }
}

void Collider::AddSphere(const std::string& name, const Vector3& offset, float radius, const Quaternion& localRotate)
{
    // 同名パーツがあれば更新、なければ追加
    for (auto& def : sphereDefs_) {
        if (def.name == name) {
            def.offset = offset;
            def.radius = radius;
            def.localRotate = localRotate;
            return;
        }
    }
    sphereDefs_.push_back({ name, offset, radius, localRotate });
}

bool Collider::SetSphereOffset(const std::string& name, const Vector3& offset)
{
    for (auto& def : sphereDefs_) {
        if (def.name == name) {
            def.offset = offset;
            return true;
        }
    }
    return false;
}

bool Collider::SetSphereRadius(const std::string& name, float radius)
{
    for (auto& def : sphereDefs_) {
        if (def.name == name) {
            def.radius = radius;
            return true;
        }
    }
    return false;
}

std::vector<Sphere> Collider::GetWorldSpheres() const
{
    std::vector<Sphere> result;
    Vector3 basePos = GetWorldPosition();

    if (sphereDefs_.empty()) {
        // デフォルトの単一球
        Sphere s;
        s.center = basePos;
        s.radius = radius_;
        s.rotate = rotation_;
        result.push_back(s);
    } else {
        // モデル形状に合わせた各パーツ球
        for (const auto& def : sphereDefs_) {
            Sphere s;
            // ローカルオフセットをオーナーの回転で旋回させて加算
            Vector3 rotatedOffset = RotateVector(def.offset, rotation_);
            s.center = Add(basePos, rotatedOffset);
            s.radius = def.radius;
            // ローカル回転とオーナー回転を合成
            s.rotate = rotation_ * def.localRotate;
            result.push_back(s);
        }
    }
    return result;
}

void Collider::Draw()
{
#ifdef USE_LINE
    auto spheres = GetWorldSpheres();
    for (const auto& sphere : spheres) {
        // プレイヤーは水色・緑系、それ以外は赤系で描画
        Vector4 col = (category_ == CollisionCategory::Player) ? Vector4{ 0.2f, 0.8f, 1.0f, 1.0f } : Vector4{ 1.0f, 0.0f, 0.0f, 1.0f };
        PrimitiveDrawer::GetInstance()->DrawSphere(sphere, col);
    }
#endif // USE_LINE
}

Vector3 Collider::GetWorldPosition() const
{
    if (owner_) {
        return owner_->GetWorldPosition() + Offset_;
    }
    return position_;
}

void Collider::OnCollision(Collider* other)
{
    if (owner_ && other && other->GetOwner()) {
        owner_->OnCollision(other->GetOwner());
    }
}

