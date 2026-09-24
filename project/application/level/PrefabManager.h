#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Vector3.h"

/// <summary>
/// ステージ配置用ブロックのプレハブ定義。
/// モデル、テクスチャ、当たり判定、すり抜け属性などのテンプレート情報を保持する。
/// </summary>
struct BlockPrefabDefinition {
    std::string prefabId;            ///< プレハブ識別名 (例: "standard_block", "oneway_platform", "floating_island")
    std::string modelName = "box";   ///< モデル名 ("box" または "xxx.obj")
    std::string modelDir = "resources"; ///< モデルディレクトリ
    std::string texturePath = "resources/grass.png"; ///< テクスチャパス
    Vector3 defaultScale = { 1.0f, 1.0f, 1.0f };    ///< デフォルトスケール
    bool isOneway = false;           ///< すり抜け足場フラグ
    bool collisionEnabled = true;    ///< コリジョン有効フラグ
    std::unordered_map<std::string, std::string> defaultProperties; ///< デフォルトプロパティ
};

/// <summary>
/// データ駆動型プレハブ管理クラス（シングルトン）。
/// 各種プレハブの登録・取得を一元管理し、レベルローダーやStageBlockのインスタンス化を支援する。
/// </summary>
class PrefabManager {
public:
    static PrefabManager* GetInstance();

    /// <summary>
    /// 初期化（組み込みプレハブの登録）
    /// </summary>
    void Initialize();

    /// <summary>
    /// プレハブの登録
    /// </summary>
    void RegisterPrefab(const BlockPrefabDefinition& def);

    /// <summary>
    /// プレハブの取得（存在しない場合は nullptr）
    /// </summary>
    const BlockPrefabDefinition* FindPrefab(const std::string& prefabId) const;

    /// <summary>
    /// プレハブが存在するか確認
    /// </summary>
    bool HasPrefab(const std::string& prefabId) const;

    /// <summary>
    /// 全プレハブ定義の取得
    /// </summary>
    const std::unordered_map<std::string, BlockPrefabDefinition>& GetAllPrefabs() const {
        return prefabs_;
    }

private:
    PrefabManager() = default;
    ~PrefabManager() = default;
    PrefabManager(const PrefabManager&) = delete;
    PrefabManager& operator=(const PrefabManager&) = delete;

    std::unordered_map<std::string, BlockPrefabDefinition> prefabs_;
    bool isInitialized_ = false;
};
