#pragma once
#include <memory>
#include <vector>

class Player;
class Enemy;

class CollisionManager {
public:
    static CollisionManager* GetInstance();
    void Finalize();

    // 判定対象の登録（毎フレームリセットする想定）
    void SetPlayer(Player* player) { player_ = player; }
    void AddEnemy(Enemy* enemy) { enemies_.push_back(enemy); }
    
    // リストのクリア
    void Clear() {
        player_ = nullptr;
        enemies_.clear();
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
};