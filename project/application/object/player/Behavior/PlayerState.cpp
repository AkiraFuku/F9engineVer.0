#include "PlayerState.h"
#include "PlayerAction.h"
#include "Command.h"
#include <memory>
#include "Player.h"
#include "PlayerBehavior.h"


 void IPlayerState::Update(Player* player){
        if (currentBehavior_) {
            currentBehavior_->Update(player);
        }
    }

void IPlayerState::HandleInput(Player* player, ICommand* command) {
    // デフォルト実装（必要に応じて派生クラスでオーバーライド）
}


//通常状態
void StateNormal::Initialize(Player* player)
{
    // 新しいビヘイビアをセットアップ
    player->ChangeBehavior(std::make_unique<BehaviorRoot>());

    if (player->GetBehavior()) {
        player->GetBehavior()->Initialize(player);
    }
    moveAction_ = std::make_unique<NormalMoveAction>(1.0f); // デフォルト速度
    jumpAction_ = std::make_unique<NormalJumpAction>();
    attackAction_ = std::make_unique<NormalAttackAction>(); // ここで生成
}

void StateNormal::Update(Player* player)
{
     if (player->GetBehavior()) {
         player->GetBehavior()->Update(player);
     }
   //  player->UpdateGravity();
}
void StateNormal::BehaviorUpdate(Player* player)
{
   player->UpdateGravity();
}

void StateNormal::Finalize(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Finalize(player);
    }
}

void StateNormal::HandleInput(Player* player, ICommand* command)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}


/*IStateRideOn::IStateRideOn(std::unique_ptr<IPlayerAction> move, std::unique_ptr<IPlayerAction> attack)
    : moveAction_(std::move(move)), attackAction_(std::move(attack)) {
}*/

IStateRideOn::IStateRideOn(std::unique_ptr<IPlayerAction> move,std::unique_ptr<IPlayerAction> jump,std::unique_ptr<IPlayerAction> attack,
        std::unique_ptr<IPlayerAction> shoot)
    : moveAction_(std::move(move)), jumpAction_(std::move(jump)), attackAction_(std::move(attack)), shootAction_(std::move(shoot)) {
}

void IStateRideOn::Update(Player* player)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->Update(player);
    }
}
void IStateRideOn::Finalize(Player* player) {
    // 1. ロボットオブジェクトを弾として生成（またはEnemyを再利用して飛ばす）
    // LaunchRobot(player->GetPosition(), shootDirection);

    // 2. プレイヤーに反動を与える
    // Vector3 recoil = shootDirection * -1.0f * kRecoilPower;
    // player->SetVelocity(recoil); 
}
void IStateRideOn::HandleInput(Player* player, ICommand* command)
{
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}

void IStateRideOn::DoMove(Player* player)
{
    if (moveAction_) moveAction_->Execute(player);
}

void IStateRideOn::DoShoot(Player* player)
{
    if (shootAction_)shootAction_->Execute(player);
}

// --- StateRideOnTest の実装（テスト用の仮のライドオンステート派生クラス） ---
StateRideOnTest::StateRideOnTest()
    : IStateRideOn(
        std::make_unique<NormalMoveAction>(0.15f),  // 移動速度
        std::make_unique<NormalJumpAction>(),       // ジャンプアクション
        std::make_unique<NormalAttackAction>(),     // 攻撃アクション
        std::make_unique<ShootRobotAction>()        // 射出アクション
    ) {
    shootAction_ = std::make_unique<ShootRobotAction>(); // 射出アクション
}

void StateRideOnTest::Initialize(Player* player) {
    // 新しいビヘイビアをセットアップ（RideOnTest専用のビヘイビア、またはRootを使用）
    player->ChangeBehavior(std::make_unique<BehaviorRoot>());

    // Behaviorの初期化
    if (player->GetBehavior()) {
        player->GetBehavior()->Initialize(player);
    }
}

void StateRideOnTest::BehaviorUpdate(Player* player) {
   player->UpdateGravity();
}

void StateRideOnTest::HandleInput(Player* player, ICommand* command) {
    // ShootCommand（射出コマンド）を処理する
    if (dynamic_cast<ShootCommand*>(command)) {
        // 射出アクションを実行
        DoShoot(player);
        // 射出後、通常状態に戻る
        player->ChangeState(std::make_unique<StateNormal>());
        return;
    }

    // その他の入力はBehaviorに任せる
    if (player->GetBehavior()) {
        player->GetBehavior()->HandleInput(player, command);
    }
}
