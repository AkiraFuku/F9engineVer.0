"""
レベルエディタ - マスターローダー (level_editor.py)
----------------------------------------------------
各機能は 'Level_editer/' サブフォルダ内の各モジュールに分割管理されています:
  - stretch_vertex.py       : 頂点編集オペレーター
  - colider.py              : コライダー設定パネル & 3Dビュー描画
  - export_scene.py         : シーンJSONエクスポート（地形グリッド対応）
  - import_scene.py         : シーンJSONインポート（地形グリッドメッシュ生成対応）
  - terrain_generator.py    : コース地形自動生成 & OBJモデルの地形グリッド変換
  - rail_snap.py            : レール進行度計算 & リアルタイム自動マグネット吸着
  - operater.py             : ファイル名設定・ICO球生成・BT起動
  - object_type.py          : オブジェクトタイプ設定 & プレビューモデル表示
  - behavior_tree_editor.py : ビヘイビアツリーノードエディタ

Blender のテキストエディタで [Run Script] を実行、またはアドオンとして有効化すると、
Level_editer 内の各モジュールが自動で読み込まれます。
"""

import sys
import os
import importlib
import traceback
import bpy

bl_info = {
    "name": "レベルエディタ",
    "author": "Fuku Akira",
    "version": (1, 2),
    "blender": (3, 3, 1),
    "location": "TopBar > MyMenu / 3D Viewport Header / Object Properties",
    "description": "ステージエディタ（地形グリッド生成・OBJ変換・レール自動吸着・シーン入出力対応）",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object",
}

# =========================================================================
# 1. Level_editer パッケージのパス探索
# アドオンフォルダに単体インストールされた場合でもプロジェクトフォルダを自動探索
# =========================================================================
_candidate_dirs = [
    os.path.dirname(os.path.abspath(__file__)),
    r"c:\Users\k024g\OneDrive\デスクトップ\study\3nenn\AL\Project\project\Editer",
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "Level_editer"),
]

_FOUND_DIR = None
for d in _candidate_dirs:
    pkg_path = os.path.join(d, "Level_editer")
    if os.path.isdir(pkg_path) and os.path.isfile(os.path.join(pkg_path, "__init__.py")):
        _FOUND_DIR = d
        break

if _FOUND_DIR:
    if _FOUND_DIR not in sys.path:
        sys.path.insert(0, _FOUND_DIR)
    print(f"[level_editor] Level_editer パッケージを検出: {_FOUND_DIR}")
else:
    # 探索失敗時は現在のスクリプトディレクトリを使用
    _CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
    if _CURRENT_DIR not in sys.path:
        sys.path.insert(0, _CURRENT_DIR)

# =========================================================================
# 2. Level_editer パッケージ内の各サブモジュールをホットリロード
# =========================================================================
_SUBMODULES = [
    "stretch_vertex",
    "colider",
    "export_scene",
    "import_scene",
    "terrain_generator",
    "road_generator",
    "stage_generator",
    "rail_snap",
    "operater",
    "object_type",
    "behavior_tree_editor",
]

for submod in _SUBMODULES:
    full_name = f"Level_editer.{submod}"
    try:
        if full_name in sys.modules:
            importlib.reload(sys.modules[full_name])
        else:
            importlib.import_module(full_name)
    except Exception as e:
        print(f"[level_editor] モジュール '{full_name}' の読み込みスキップ/エラー: {e}")

# =========================================================================
# 3. Level_editer パッケージ自体のリロード
# =========================================================================
level_editer_pkg = None
try:
    if "Level_editer" in sys.modules:
        level_editer_pkg = importlib.reload(sys.modules["Level_editer"])
    else:
        level_editer_pkg = importlib.import_module("Level_editer")
    print("[level_editor] Level_editer パッケージのインポート/リロードに成功しました")
except Exception as e:
    print(f"[level_editor] Level_editer パッケージのリロードエラー: {e}")
    traceback.print_exc()


def register():
    global level_editer_pkg
    # 未ロードの場合は再試行
    if not level_editer_pkg:
        try:
            if "Level_editer" in sys.modules:
                level_editer_pkg = importlib.reload(sys.modules["Level_editer"])
            else:
                level_editer_pkg = importlib.import_module("Level_editer")
        except Exception as e:
            print(f"[level_editor] register 時の再インポート失敗: {e}")

    if level_editer_pkg and hasattr(level_editer_pkg, "register"):
        try:
            level_editer_pkg.unregister()
        except Exception:
            pass
        level_editer_pkg.register()
    else:
        print("[level_editor] エラー: Level_editer パッケージが読み込まれていないため登録できません")


def unregister():
    if level_editer_pkg and hasattr(level_editer_pkg, "unregister"):
        try:
            level_editer_pkg.unregister()
        except Exception:
            pass


if __name__ == "__main__":
    register()
