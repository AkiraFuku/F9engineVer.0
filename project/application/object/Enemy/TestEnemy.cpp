#include "TestEnemy.h"
#include "TestRobot.h"
#include "Robot.h"
#include "Player.h"
#include "PlayerState.h"
#include "EnemyState.h"

TestEnemy::TestEnemy() {
}

TestEnemy::~TestEnemy() = default;

void TestEnemy::Initialize() {
    Enemy::Initialize();

    // 基底クラスのメソッドを使ってロボットを登録するだけ
    SetRobot(std::make_unique<TestRobot>());
}

void TestEnemy::Update() {
    Enemy::Update();

    // ロボット自体は更新が必要ないため、スキップ
}

void TestEnemy::Draw() {
    Enemy::Draw();

    // ロボット自体は描画が必要ないため、スキップ
}

void TestEnemy::OnStun() {
    // スタン状態になったときの処理
    // 必要に応じてオーバーライド可能
}

