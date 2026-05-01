#pragma once
#include <memory>
#include <vector>

class Player;
class Enemy;
class Projectile; // 前方宣言

class CollisionManager {
public:
    static CollisionManager* GetInstance();
    void Finalize();

    // 判定対象の登録（毎フレームリセットする想定）
    void SetPlayer(Player* player) { player_ = player; }
    void AddEnemy(Enemy* enemy) { enemies_.push_back(enemy); }
    void AddProjectile(Projectile* projectile) { projectiles_.push_back(projectile); }
    
    // リストのクリア
    void Clear() {
        player_ = nullptr;
        enemies_.clear();
        projectiles_.clear(); // 弾リストもクリア
    }

    // 全ての衝突チェックを実行
    void CheckAllCollisions();

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;

    static std::unique_ptr<CollisionManager> instance;
     friend struct std::default_delete<CollisionManager>;

    Player* player_ = nullptr;
    std::vector<Enemy*> enemies_;

    // 2点間の球判定（必要に応じて引数を調整してください）
    void CheckPlayerEnemyCollision(Player* p, Enemy* e);

    std::vector<Projectile*> projectiles_;
    
    // 弾と敵の衝突判定
    void CheckProjectileEnemyCollision(Projectile* p, Enemy* e);
    // 弾とプレイヤーの衝突判定（敵の弾用）
    void CheckProjectilePlayerCollision(Projectile* p, Player* player);
};