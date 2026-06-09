#pragma once
#include "ICollider.h"
#include "Object3d.h"
#include <memory>
#include "Camera.h"
class Terrain :
    public ICollider
{

public:
    void Initialize(const std::string& modelPath);
    void Update();
    void Draw();

    // ICollider の実装
    void OnCollision(ICollider* other) override;
    Vector3 GetWorldPosition() const override {
        return object_->GetTranslate();
    }
    CollisionCategory GetCategory() const override {
        return CollisionCategory::Terrain;
    }

    void UpdateBoundingBox(Vector3 max, Vector3 min) override;
    bool TestCollision(const ICollider* other) const override;

    // 地面との高さ判定用：レイキャストの実装
  //  bool TestRay(const Ray& ray, RayCastHit* outHit) override;

    void SetPosition(const Vector3& pos) {
        object_->SetTranslate(pos);
    }
    void SetScale(const Vector3& scale) {
        object_->SetScale(scale);
    }
    void SetRotate(const Vector3& rotate) {
        object_->SetRotate(rotate);
    }
    void SetCamera(Camera* camera) {
        if (object_) {
            object_->SetCamera(camera);
        }
    }

    Object3d* GetObject3d() const {
        return object_.get();
    }

private:
    std::unique_ptr<Object3d> object_;

};

