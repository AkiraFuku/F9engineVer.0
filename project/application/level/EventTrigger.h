#pragma once
#include "LevelData.h"
#include "Vector3.h"
#include "MathFunction.h"
#include <string>
#include <functional>
#include <unordered_map>

/// <summary>
/// イベントトリガークラス。
/// AABB (Box) または Sphere の不可視判定領域を持ち、
/// プレイヤーなどの座標との交差を毎フレーム判定する。
/// 侵入/退出・継続・1回限りの3モードをサポート。
/// </summary>
class EventTrigger
{
public:
    /// 発火モード
    enum class FireMode {
        Once,       ///< 初めて入ったときのみ1回発火
        Continuous, ///< 領域内にいる間、毎フレーム発火
        EnterExit,  ///< 入ったとき(Enter) & 出たとき(Exit)に発火
    };

    /// コールバック型（イベント名とパラメータを受け取る）
    using TriggerCallback = std::function<void(const std::string& eventName,
                                               const std::unordered_map<std::string, std::string>& params)>;

    EventTrigger() = default;
    ~EventTrigger() = default;

    /// <summary>
    /// LevelObjectData から初期化
    /// </summary>
    void Initialize(const LevelObjectData& data);

    /// <summary>
    /// 毎フレームの判定更新。
    /// queryPos に対象オブジェクト（プレイヤー等）の座標を渡す。
    /// 侵入/退出イベントが発生した場合、登録済みコールバックを呼ぶ。
    /// </summary>
    void Update(const Vector3& queryPos);

    // ─── セッター ──────────────────────────────────────────────────
    /// 発火時に呼ばれるコールバックを設定する
    void SetCallback(TriggerCallback callback) { callback_ = std::move(callback); }
    void SetDisabled(bool disabled) { disabled_ = disabled; }
    void Reset() { hasFired_ = false; isInsidePrev_ = false; }

    // ─── ゲッター ──────────────────────────────────────────────────
    bool IsDisabled()   const { return disabled_; }
    bool HasFired()     const { return hasFired_; }
    bool IsInsideNow()  const { return isInsideNow_; }
    const std::string& GetName()      const { return name_; }
    const std::string& GetEventName() const { return eventName_; }

    /// デバッグ描画（ワイヤーフレームBOX/SPHERE）
    void DebugDraw() const;

private:
    /// 座標が判定領域内にあるか
    bool ContainsPoint(const Vector3& point) const;

    std::string     name_       = "";
    std::string     eventName_  = "";
    FireMode        fireMode_   = FireMode::Once;
    bool            disabled_   = false;
    bool            hasFired_   = false;      ///< Once モードで発火済みフラグ
    bool            isInsidePrev_ = false;    ///< 前フレームでの侵入状態
    bool            isInsideNow_  = false;    ///< 今フレームでの侵入状態

    // 形状定義（LevelColliderData を流用）
    LevelColliderData::Shape shape_ = LevelColliderData::Shape::Box;
    Vector3 worldCenter_ = { 0.0f, 0.0f, 0.0f }; ///< ワールド中心
    Vector3 halfExtents_ = { 1.0f, 1.0f, 1.0f }; ///< Box: 半辺長 / Sphere: x=半径

    /// イベント発火時コールバック
    TriggerCallback callback_ = nullptr;

    /// イベントに渡すカスタムパラメータ
    std::unordered_map<std::string, std::string> params_;
};
