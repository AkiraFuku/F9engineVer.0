#pragma once
#include "LevelData.h"
#include <string>

/// <summary>
/// Blenderアドオン（Level_editer）が出力するJSONファイルを読み込み、
/// LevelData 構造体へパースするローダークラス。
///
/// 座標変換:
///   Blender 右手系 Z-up   → DirectX 左手系 Y-up
///   Translation : (X, Y, Z) → (X, Z, Y)
///   Rotation    : Blender 度数法オイラー → ラジアン変換後、軸マッピング
///   Scale       : (X, Y, Z) → (X, Z, Y)
/// </summary>
class LevelLoader
{
public:
    /// <summary>
    /// JSONファイルを読み込み LevelData を返す。
    /// 失敗時は空の LevelData を返す（エラーはログ出力）。
    /// </summary>
    static LevelData Load(const std::string& filePath);
};
