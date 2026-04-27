# StateRideOnTest 検証ガイド

## 概要
`StateRideOnTest`は、ライドオンステートの派生クラスで、プレイヤーアクション（射出）の検証を行うための仮実装です。

## 実装内容

### 1. **StateRideOnTest クラス** (PlayerState.h/cpp)
- `IStateRideOn`の派生クラス
- 移動アクション、攻撃アクション、射出アクションを初期化

### 2. **ShootCommand クラス** (Command.h)
- 射出アクションをトリガーするためのコマンド
- `StateRideOnTest::HandleInput()`で処理

### 3. **InputHandler の拡張** (InputHandler.cpp)
- **Zキー** または **ゲームパッドXボタン** で射出コマンドを生成

## 検証手順

### 1. ステート切り替え
GameSceneのDebugウィンドウから以下のボタンを使用：

```
- "Switch to RideOnTest State"     → StateRideOnTestに切り替え
- "Switch to Normal State"          → StateNormalに戻す
```

### 2. 射出アクションの実行
StateRideOnTestに切り替え後、以下を入力：

| 入力 | アクション |
|------|----------|
| **Z キー** | 射出実行 → 弾生成 → 自動的にNormalStateに戻す |
| **ゲームパッド X ボタン** | 同上 |

### 3. 検証項目

✅ **射出実行時に弾が生成されているか**
- GameScene内の`projectiles_`ベクトルに弾が追加される
- 弾がレール上を移動する

✅ **射出後、StateNormalに戻すか**
- 射出直後にステートが自動的に通常状態に切り替わる
- ImGuiの「Base State」が「Normal」に変わることで確認可能

✅ **弾が正しく管理されているか**
- Update時に死んだ弾が削除される
- Draw時に弾が描画される

## 実装フロー

```
射出入力（Z または ゲームパッド X）
    ↓
InputHandler::HandleInput() → ShootCommand生成
    ↓
Player::Update() → StateRideOnTest::HandleInput()
    ↓
ShootCommand検出 → DoShoot()実行
    ↓
ShootRobotAction::Execute()
    ↓
Player::GetScene() → GameScene::AddProjectile()
    ↓
Projectile生成・管理
    ↓
Player::ChangeState(StateNormal) → 通常状態に戻す
```

## コード例

### ステート切り替え（GameScene内）
```cpp
if (ImGui::Button("Switch to RideOnTest State")) {
    player->ChangeState(std::make_unique<StateRideOnTest>());
}
```

### 射出入力処理（StateRideOnTest内）
```cpp
void StateRideOnTest::HandleInput(Player* player, ICommand* command) {
    if (dynamic_cast<ShootCommand*>(command)) {
        DoShoot(player);  // 射出実行
        player->ChangeState(std::make_unique<StateNormal>());  // 戻す
        return;
    }
    // ... その他の入力処理
}
```

## トラブルシューティング

### 弾が生成されない場合
1. `Player::SetScene(this)`がGameScene::Initializeで呼ばれているか確認
2. `GameScene::AddProjectile()`の実装を確認
3. Projectileクラスのカメラ設定を確認

### ステートが戻らない場合
1. `StateRideOnTest::HandleInput()`でShootCommand検出後に`ChangeState()`が呼ばれているか確認
2. ImGuiで実際にNormalStateに切り替わっているか確認

### 入力が反応しない場合
1. `InputHandler.cpp`でShootCommandが生成されているか確認
2. Z キーまたはゲームパッド X ボタンが正しく設定されているか確認

## 次のステップ

実装が確認できたら：
1. 具体的なロボットモデルの搭乗ステート実装
2. 射出時のエフェクト・音声
3. 搭乗キャンセル機能
