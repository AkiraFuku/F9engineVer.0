#pragma once
#include "Enemy.h"
#include <vector>
class Cllider;
class BossPart;

class MiniBoss {
public:
    MiniBoss() ;
    ~MiniBoss()  ;

    void Initialize() ;
    void Update() ;
    void Draw() ;

    void TakeDamage(int damage); // ボス全体のHP減算などのインターフェース
    // シーンから全パーツのコライダーを登録できるように公開
    const std::vector<std::unique_ptr<BossPart>>& GetParts() const { return parts_; }

    std::vector<Collider*> GetColliders() const;
  //  void OnCollision(GameObject* other) override;
    Vector3 GetPosition() const {
        return position_;
    }
private:
    std::vector<std::unique_ptr<BossPart>> parts_;
    int hp_ = 5;

    bool isDamaged_ = false;
    Vector3 position_ = {};

};