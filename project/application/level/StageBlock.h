#pragma once
#include "Object3d.h"
#include "Camera.h"
#include "LevelData.h"
#include "MathFunction.h"
#include <memory>
#include <string>
#include <vector>

/// <summary>
/// 接地判定・壁判定を持つ汎用ステージブロック（プレハブ）クラス。
/// Blenderレベルエディターの kBlock / kTerrain タイプに対応。
/// Object3d を内包し、自身のワールドポリゴンを抽出して
/// GameScene の triangles_ リストへ提供する。
/// </summary>
class StageBlock
{
public:
    StageBlock() = default;
    ~StageBlock() = default;

    /// <summary>
    /// LevelObjectData から初期化する（レベルローダーから呼ばれる）
    /// </summary>
    void Initialize(const LevelObjectData& data, Camera* camera);

    /// <summary>
    /// 毎フレームの更新（ワールド三角形の再計算など）
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    // ─── セッター ─────────────────────────────────────────────────────
    void SetCamera(Camera* camera) { if (object_) object_->SetCamera(camera); }
    void SetDisabled(bool disabled) { disabled_ = disabled; }
    void SetCollisionEnabled(bool enabled) { isCollisionEnabled_ = enabled; }
    void SetTranslate(const Vector3& pos) { if (object_) object_->SetTranslate(pos); }
    void SetRotate(const Vector3& rot)    { if (object_) object_->SetRotate(rot); }
    void SetScale(const Vector3& scl)     { if (object_) object_->SetScale(scl); }

    // ─── ゲッター ─────────────────────────────────────────────────────
    bool IsDisabled()          const { return disabled_; }
    bool IsCollisionEnabled()  const { return isCollisionEnabled_; }
    const std::string& GetName() const { return name_; }
    LevelObjectType GetType()  const { return type_; }

    /// <summary>
    /// 接地/壁判定に使用するワールド三角形リストを返す。
    /// disabled_ または isCollisionEnabled_ == false の場合は空リストを返す。
    /// </summary>
    const std::vector<Triangle>& GetWorldTriangles() const { return triangles_; }

    Object3d* GetObject() const { return object_.get(); }

private:
    std::unique_ptr<Object3d> object_ = nullptr;

    std::string     name_               = "";
    LevelObjectType type_               = LevelObjectType::kBlock;
    bool            disabled_           = false;  ///< 無効化フラグ（trueなら更新・描画・判定をスキップ）
    bool            isCollisionEnabled_ = true;   ///< false にすると三角形判定を提供しない

    /// キャッシュされたワールド空間三角形リスト
    mutable std::vector<Triangle> triangles_;
};
