#pragma once
#include "Enemy.h"
#include <vector>

enum class BossPartType {
    Body,   // めり込めない胴体
    Weak,   // 弱点（攻撃可能）
    Armor   // 攻撃不可（装甲）めり込まない
};

struct BossPart {
    BossPartType type;
    std::unique_ptr<Object3d> object;
    std::unique_ptr<Collider> collider;
    Vector3 localPosition;
};

class MiniBoss : public Enemy {
public:
    MiniBoss() = default;
    ~MiniBoss() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    // シーンから全パーツのコライダーを登録できるように公開
    const std::vector<std::unique_ptr<BossPart>>& GetParts() const { return parts_; }

    void OnCollision(GameObject* other) override;

private:
    std::vector<std::unique_ptr<BossPart>> parts_;
    int hp_ = 5;
};