#pragma once
#include "Enemy.h"
#include <vector>
class Cllider;
class BossPart;

class MiniBoss : public Enemy {
public:
    MiniBoss() ;
    ~MiniBoss() override ;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void TakeDamage(int damage); // ボス全体のHP減算などのインターフェース
    // シーンから全パーツのコライダーを登録できるように公開
    const std::vector<std::unique_ptr<BossPart>>& GetParts() const { return parts_; }

    std::vector<Collider*> GetColliders() const;
    void OnCollision(GameObject* other) override;

private:
    std::vector<std::unique_ptr<BossPart>> parts_;
    int hp_ = 5;
};