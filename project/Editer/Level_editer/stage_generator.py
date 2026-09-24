import bpy
import bmesh
import math
import mathutils
import random
import os
import time

# =========================================================================
# 星のカービィ64風 AIステージ自動生成・伸長モジュール (stage_generator.py)
#
# 【特徴】
# 1. 既存の StageRail の終端から、滑らかなベジェ曲線でコースをプロシージャル自動伸長。
# 2. カービィ64らしい「直線」「緩やかなカーブ」「直角90°コーナー」「S字」「上り坂/下り坂」「ジャンプ穴」「浮遊足場」の構成パターンをAI生成。
# 3. レールに沿って自動で向きを合わせた足場ブロック（BLOCK）を配置（穴・段差対応）。
# 4. エネミー（Normal / Bound）を要所（穴の手前、直線、高台）に自動配置。
# 5. カメラレール（CameraRail）の自動追従同期、およびゴール（GOAL）の終端移動。
# 6. ランダムシード、難易度・テーマ、セクション数の自由なパラメータ調整とUndo対応。
# =========================================================================


def _get_project_root():
    """プロジェクトのルートディレクトリパスを取得"""
    cur_file = os.path.abspath(__file__)
    editer_dir = os.path.dirname(os.path.dirname(cur_file))
    return os.path.dirname(editer_dir)


def find_stage_rail():
    """シーン内の StageRail オブジェクトを取得"""
    for name in ("StageRail", "stageRail", "stage_rail"):
        if name in bpy.data.objects:
            obj = bpy.data.objects[name]
            if obj.type == 'CURVE':
                return obj
    for obj in bpy.data.objects:
        if obj.type == 'CURVE' and obj.get("object_type") == "STAGE_RAIL":
            return obj
    for obj in bpy.data.objects:
        if obj.type == 'CURVE':
            return obj
    return None


def get_or_create_stage_collection():
    """ステージ生成用コレクションを取得または作成（シーン整理用）"""
    col_name = "AI_Generated_Stage"
    if col_name in bpy.data.collections:
        return bpy.data.collections[col_name]
    col = bpy.data.collections.new(col_name)
    bpy.context.scene.collection.children.link(col)
    return col


# =========================================================================
# 1. レール延長・幾何計算ロジック
# =========================================================================

class CurveExtensionHelper:
    """ベジェ曲線の終端からスムーズにセグメントを追加するヘルパー"""

    def __init__(self, curve_obj):
        self.curve_obj = curve_obj
        self.curve_data = curve_obj.data
        if not self.curve_data.splines:
            self.spline = self.curve_data.splines.new('BEZIER')
            self.spline.use_cyclic_u = False
        else:
            self.spline = self.curve_data.splines[0]

        # 延長前の既存セグメント数を基準値として保持
        self.base_seg_index = max(0, len(self.spline.bezier_points) - 1)

        # 最後の点と進行方向を取得
        self._init_endpoint()

    def _init_endpoint(self):
        bp = self.spline.bezier_points
        if len(bp) == 0:
            # 最初の点を原点に初期化
            bp.add(1)
            p0 = bp[0]
            p0.co = mathutils.Vector((0.0, 0.0, 0.0))
            p0.handle_left = mathutils.Vector((0.0, -2.0, 0.0))
            p0.handle_right = mathutils.Vector((0.0, 2.0, 0.0))
            p0.handle_left_type = 'ALIGNED'
            p0.handle_right_type = 'ALIGNED'
            self.current_pos = p0.co.copy()
            self.current_fwd = mathutils.Vector((0.0, 1.0, 0.0))  # +Y方向へ進行
        elif len(bp) == 1:
            p0 = bp[0]
            self.current_pos = p0.co.copy()
            tan = p0.handle_right - p0.co
            if mathutils.Vector((tan.x, tan.y, 0.0)).length > 1e-4:
                self.current_fwd = mathutils.Vector((tan.x, tan.y, 0.0)).normalized()
            else:
                self.current_fwd = mathutils.Vector((0.0, 1.0, 0.0))
        else:
            p_last = bp[-1]
            p_prev = bp[-2]
            self.current_pos = p_last.co.copy()
            tan = p_last.co - p_prev.co
            if (p_last.handle_right - p_last.co).length > 1e-4:
                tan = p_last.handle_right - p_last.co
            # 水平進行方向を基本とする
            h_tan = mathutils.Vector((tan.x, tan.y, 0.0))
            if h_tan.length > 1e-4:
                self.current_fwd = h_tan.normalized()
            else:
                self.current_fwd = mathutils.Vector((0.0, 1.0, 0.0))

        self.base_z = self.current_pos.z

    def append_point(self, new_pos, new_fwd, handle_length=None, interp_type='BEZIER'):
        """新しい制御点を滑らかに追加（interp_type: 'LINEAR' または 'BEZIER'）"""
        bp = self.spline.bezier_points
        last_idx = len(bp) - 1
        last_p = bp[last_idx]

        dist = (new_pos - self.current_pos).length
        if handle_length is None:
            handle_length = dist * 0.35

        # 新しい制御点を追加
        bp.add(1)
        new_p = bp[-1]
        new_p.co = new_pos.copy()

        if interp_type == 'LINEAR':
            # 直線補間: ハンドルタイプを VECTOR に設定
            new_p.handle_left_type = 'VECTOR'
            new_p.handle_right_type = 'VECTOR'
            if last_p.handle_left_type == 'VECTOR' or last_idx == 0:
                last_p.handle_right_type = 'VECTOR'
        else:
            # 曲線補間: 前の点の出ハンドルと新しい点の入ハンドルを接線方向に設定
            last_p.handle_right = last_p.co + self.current_fwd * handle_length
            last_p.handle_right_type = 'ALIGNED'
            new_p.handle_left = new_pos - new_fwd * handle_length
            new_p.handle_right = new_pos + new_fwd * handle_length
            new_p.handle_left_type = 'ALIGNED'
            new_p.handle_right_type = 'ALIGNED'

        self.current_pos = new_pos.copy()
        self.current_fwd = new_fwd.normalized()


# =========================================================================
# 2. 星のカービィ64風 コースパターンジェネレータ
# =========================================================================

# プレハブ定義（足場サイズ・属性・テクスチャのテンプレート）
_BLOCK_PREFABS = {
    "standard": {
        "size": (4.0, 3.15, 1.0),
        "texture": "resources/grass.png",
        "is_oneway": False,
    },
    "oneway_platform": {
        "size": (3.5, 2.0, 0.2), # 薄型のすり抜け足場
        "texture": "resources/grass.png",
        "is_oneway": True,
    },
    "floating_island": {
        "size": (6.5, 5.0, 1.5), # 広めの浮島ベース
        "texture": "resources/grass.png",
        "is_oneway": False,
    },
}

class StageAIGenerator:
    """
    カービィ64風のコース構造（レール、足場、敵）をプロシージャル生成するエンジン
    """

    def __init__(self, seed=None):
        self.rng = random.Random(seed)

    def generate(self, context, settings):
        """
        設定に基づいてコースを伸長・自動生成
        """
        rail_obj = find_stage_rail()
        if not rail_obj:
            # StageRailが存在しない場合は新規作成
            curve_data = bpy.data.curves.new(name="StageRail", type='CURVE')
            curve_data.dimensions = '3D'
            curve_data.resolution_u = 12
            rail_obj = bpy.data.objects.new("StageRail", curve_data)
            rail_obj["object_type"] = "STAGE_RAIL"
            context.collection.objects.link(rail_obj)

        reset_existing = settings.get("reset_existing", False)
        target_col = get_or_create_stage_collection()

        # 既存コースのリセット処理（新規生成時）
        if reset_existing:
            # 既存の生成オブジェクト（足場、敵、地面など）をコレクションから削除
            for obj in list(target_col.objects):
                if obj != rail_obj and obj.name != "CameraRail":
                    bpy.data.objects.remove(obj, do_unlink=True)

            # StageRailの既存ポイントをリセット（原点0,0,0の初期状態に戻す）
            if rail_obj and rail_obj.type == 'CURVE' and rail_obj.data.splines:
                rail_obj.data.splines.clear()
                sp = rail_obj.data.splines.new('BEZIER')
                sp.use_cyclic_u = False
                sp.bezier_points.add(1)
                p0 = sp.bezier_points[0]
                p0.co = mathutils.Vector((0.0, 0.0, 0.0))
                p0.handle_left = mathutils.Vector((0.0, -2.0, 0.0))
                p0.handle_right = mathutils.Vector((0.0, 2.0, 0.0))
                p0.handle_left_type = 'ALIGNED'
                p0.handle_right_type = 'ALIGNED'
                rail_obj.data.update_tag()

        helper = CurveExtensionHelper(rail_obj)

        # 生成セグメント数
        num_sections = settings.get("section_count", 12)
        theme = settings.get("theme", "BALANCED")
        environment = settings.get("environment", "PLAINS")
        camera_side = settings.get("camera_side", "RIGHT")
        curve_prob = settings.get("curve_intensity", 0.5)
        slope_prob = settings.get("slope_intensity", 0.4)
        gap_prob = settings.get("gap_density", 0.3)
        enemy_prob = settings.get("enemy_density", 0.5)
        spawn_blocks = settings.get("spawn_blocks", True)
        update_camera = settings.get("update_camera", True)
        move_goal = settings.get("move_goal", True)

        created_objects = []
        section_infos = []

        # テーマによる重み補正
        if theme == "BEGINNER":
            curve_prob *= 0.3
            slope_prob = 0.0
            gap_prob = 0.0
            enemy_prob *= 0.5
        elif theme == "ROLLER_COASTER":
            curve_prob *= 1.3
            slope_prob *= 1.3
            gap_prob *= 0.5
        elif theme == "CHALLENGE":
            gap_prob *= 1.5
            enemy_prob *= 1.3

        # 基準高さ（平原では常に水平フラットを維持）
        base_z = getattr(helper, "base_z", helper.current_pos.z)

        # 直線主体シーケンス制御:
        # 初期の直線カウントを1〜2に設定（カーブが適切に挟まり、単調な直線ばかりになるのを防止）
        straight_count = self.rng.randint(1, 2)
        last_turn = None

        # セクションを連続して生成
        for sec_i in range(num_sections):
            start_pos = helper.current_pos.copy()
            start_fwd = helper.current_fwd.copy()

            # 平原ステージではZ方向のブレや沈み込みを完全に排除（常に基準高さを維持）
            if environment == "PLAINS":
                start_pos.z = base_z
                h_fwd = mathutils.Vector((start_fwd.x, start_fwd.y, 0.0))
                start_fwd = h_fwd.normalized() if h_fwd.length > 1e-4 else mathutils.Vector((0.0, 1.0, 0.0))

            # セクションタイプの決定（最初の2セクションは安全直線、最後もゴール前直線）
            if sec_i < 2 or sec_i == num_sections - 1:
                sec_type = "STRAIGHT"
            elif straight_count > 0:
                # 快適に走れるまとまった直線区間
                sec_type = "STRAIGHT"
                straight_count -= 1
            else:
                # 直線が十分続いたので、アクセントとしてカーブ/コーナーを1区間挟む
                candidates = []
                if curve_prob > 0.35:
                    # 90°コーナー
                    candidates.extend(["CORNER_LEFT_90", "CORNER_RIGHT_90"])
                # 緩やかな30°〜45°カーブ
                candidates.extend(["CURVE_LEFT", "CURVE_RIGHT", "CURVE_LEFT", "CURVE_RIGHT"])
                if curve_prob > 0.6:
                    candidates.append("S_CURVE")

                # 浮遊アスレチックモードのみ坂道・階段を含める（平原では地面を活かすため平坦を優先）
                if environment == "PLATFORM" and slope_prob > 0.4:
                    candidates.extend(["SLOPE_UP", "SLOPE_DOWN", "SLOPE_UP_STEP"])

                chosen = self.rng.choice(candidates)
                # 連続で同じ方向に直角に曲がってループするのを抑制
                if last_turn and chosen == last_turn and self.rng.random() < 0.7:
                    chosen = "CURVE_RIGHT" if "LEFT" in chosen else "CURVE_LEFT"

                sec_type = chosen
                if "LEFT" in sec_type:
                    last_turn = "LEFT"
                elif "RIGHT" in sec_type:
                    last_turn = "RIGHT"

                # カーブを抜けたら、新しい向きで再び1〜3セクション直線が続く！
                straight_count = self.rng.randint(1, 3)

            # スタート地点周辺（sec_i < 2）およびゴール前は落とし穴確率を0にして安全を保証
            sec_gap_prob = 0.0 if sec_i < 2 or sec_i == num_sections - 1 else gap_prob

            # 終端位置と進行方向を計算（カーブ・コーナーは中間点を含む円弧ステップ列を返す）
            sub_points, sec_meta = self._compute_section_geometry(
                start_pos, start_fwd, sec_type, base_z=base_z, environment=environment, gap_prob=sec_gap_prob
            )

            sec_interp = "LINEAR" if sec_type == "STRAIGHT" else "BEZIER"
            # レールに各制御点を順次追加（直線はLINEAR、カーブはBEZIER円弧）
            for pt_pos, pt_fwd, pt_hlen in sub_points:
                helper.append_point(pt_pos, pt_fwd, handle_length=pt_hlen, interp_type=sec_interp)

            sec_meta["start_pos"] = start_pos
            sec_meta["end_pos"] = sub_points[-1][0]
            sec_meta["start_fwd"] = start_fwd
            sec_meta["end_fwd"] = sub_points[-1][1]
            sec_meta["sec_type"] = sec_type
            section_infos.append(sec_meta)

        # 🕳️ 落とし穴の確定保証（gap_prob > 0 の場合、コース途中に必ず最低1つ落とし穴をセット）
        if gap_prob > 0.0 and len(section_infos) >= 4:
            has_gap = any(s.get("is_gap", False) for s in section_infos)
            if not has_gap:
                # スタート安全区間（i < 2）と最後（ゴール前）以外の直線セクションから選ぶ
                mid_straights = [
                    s for i, s in enumerate(section_infos)
                    if 1 < i < len(section_infos) - 1 and s.get("sec_type") == "STRAIGHT"
                ]
                if mid_straights:
                    self.rng.choice(mid_straights)["is_gap"] = True
                else:
                    mid_idx = len(section_infos) // 2
                    if mid_idx >= 2:
                        section_infos[mid_idx]["is_gap"] = True

        # カーブの更新通知
        rail_obj.data.update_tag()

        # 直線・カーブに応じた補間タイプリストを StageRail に設定
        try:
            from .rail_snap import set_curve_interp_types
            pts = rail_obj.data.splines[0].bezier_points
            types_list = []
            for bp in pts:
                if bp.handle_left_type == 'VECTOR' and bp.handle_right_type == 'VECTOR':
                    types_list.append("LINEAR")
                else:
                    types_list.append("BEZIER")
            set_curve_interp_types(rail_obj, types_list)
        except Exception as ex:
            print(f"[stage_generator] 補間タイプ設定エラー: {ex}")

        # 🌿 平原ステージ (PLAINS) の場合、コース全体を包み込む草原地面メッシュ (terrain_grid) を自動配置/更新
        if environment == "PLAINS":
            ground_obj = self._ensure_plains_ground(context, rail_obj, target_col, section_infos=section_infos)
            if ground_obj and ground_obj not in created_objects:
                created_objects.append(ground_obj)

            # ⛰️ 生成時にレールのカーブや起伏に合わせて地面を自動変形造形
            if settings.get("deform_terrain_auto", True) and ground_obj:
                context.view_layer.update()
                try:
                    from .terrain_generator import deform_terrain_mesh_to_rail
                    deform_terrain_mesh_to_rail(
                        terrain_obj=ground_obj,
                        rail_obj=rail_obj,
                        road_width=4.0,
                        slope_width=4.0,
                        offset_z=-0.1,
                        falloff="SMOOTH",
                        auto_export_obj=False
                    )
                except Exception:
                    try:
                        import terrain_generator
                        terrain_generator.deform_terrain_mesh_to_rail(
                            terrain_obj=ground_obj,
                            rail_obj=rail_obj,
                            road_width=4.0,
                            slope_width=4.0,
                            offset_z=-0.1,
                            falloff="SMOOTH",
                            auto_export_obj=False
                        )
                    except Exception as ex:
                        print(f"[stage_generator] 地面自動変形スキップ/エラー: {ex}")

                # PLAINS モードでは落とし穴（穴あけ）は行わず、平面Terrainとして維持


                # ✂️ レール（進行ルート）から離れたゲーム中に見えない不要メッシュを一括削除
                self._trim_unseen_terrain_mesh(ground_obj, rail_obj, max_distance=24.0)
                context.view_layer.update()

        # 足場ブロックと敵の配置
        if spawn_blocks or enemy_prob > 0.0 or any(s.get("is_gap", False) for s in section_infos):
            blocks, enemies = self._populate_stage(
                context, rail_obj, section_infos,
                base_seg_index=helper.base_seg_index,
                environment=environment,
                spawn_blocks=spawn_blocks,
                enemy_prob=enemy_prob,
                gap_prob=gap_prob,
                collection=target_col
            )
            created_objects.extend(blocks)
            created_objects.extend(enemies)

        # カメラレールの同期（進行方向の右側 = flip_side=False, 左側 = flip_side=True）
        if update_camera:
            flip = (camera_side == "LEFT")
            try:
                from .rail_snap import sync_camera_rail_from_stage
                sync_camera_rail_from_stage(rail_obj, flip_side=flip)
            except Exception as e:
                try:
                    import rail_snap
                    rail_snap.sync_camera_rail_from_stage(rail_obj, flip_side=flip)
                except Exception as ex:
                    print(f"[stage_generator] カメラレール同期エラー: {ex}")

        # 🏃 プレイヤー開始位置（PlayerSpawn）の配置または更新（レール始点 t=0.0）
        player_obj = self._handle_player_spawn_position(context, rail_obj, target_col)
        if player_obj and player_obj not in created_objects:
            created_objects.append(player_obj)

        # ゴール（GOAL）の移動または作成
        if move_goal:
            goal_obj = self._handle_goal_position(context, rail_obj, helper.current_pos, helper.current_fwd, target_col)
            if goal_obj and goal_obj not in created_objects:
                created_objects.append(goal_obj)

        return len(section_infos), len(created_objects)

    def _compute_section_geometry(self, pos, fwd, sec_type, base_z=None, environment="PLAINS", gap_prob=0.3):
        """
        セクションの幾何形状を計算。
        カーブ・コーナー区間では2点間の直線にならず、中間点を含む滑らかな円弧カーブレール（sub_points列）を返す。
        戻り値: (sub_points, meta)
        sub_points は [(point_pos, forward_vec, handle_length), ...] のリスト
        """
        meta = {"is_gap": False, "high_platform": False}
        up = mathutils.Vector((0.0, 0.0, 1.0))

        # 水平進行方向（Z成分は常に0にし、下り・上りの傾き累積を完全排除）
        h_fwd = mathutils.Vector((fwd.x, fwd.y, 0.0))
        if h_fwd.length < 1e-4:
            h_fwd = mathutils.Vector((0.0, 1.0, 0.0))
        h_fwd.normalize()
        right = h_fwd.cross(up).normalized()

        if base_z is None:
            base_z = pos.z

        sub_points = []

        if sec_type == "STRAIGHT":
            # 快適に走れる真っ直ぐな直線区間（約14〜20m）
            length = self.rng.uniform(14.0, 20.0)
            end_pos = pos + h_fwd * length
            if environment == "PLAINS":
                end_pos.z = base_z
            end_fwd = h_fwd.copy()
            h_len = length * 0.35

            # 落とし穴判定（平原・アスレチック双方で有効）
            if self.rng.random() < gap_prob:
                meta["is_gap"] = True

            sub_points.append((end_pos, end_fwd, h_len))

        elif sec_type == "CORNER_LEFT_90":
            # 🌀 カービィ64風の滑らかな直角90°左円弧カーブレール
            # 始点(0°) → 中間点(45°) → 終点(90°) の2ステップ分割で完璧な円弧を描く
            radius = self.rng.uniform(12.0, 16.0)
            center = pos - right * radius
            center.z = base_z if environment == "PLAINS" else pos.z

            # 45度円弧の理想的なベジェハンドル長
            h_len = radius * 0.276

            # 中間点 (45°)
            rot45 = mathutils.Matrix.Rotation(math.radians(45), 3, 'Z')
            p_mid = center + (rot45 @ (right * radius))
            f_mid = (rot45 @ h_fwd).normalized()
            if environment == "PLAINS":
                p_mid.z = base_z
            sub_points.append((p_mid, f_mid, h_len))

            # 終点 (90°)
            rot90 = mathutils.Matrix.Rotation(math.radians(90), 3, 'Z')
            p_end = center + (rot90 @ (right * radius))
            f_end = (rot90 @ h_fwd).normalized()
            if environment == "PLAINS":
                p_end.z = base_z
            sub_points.append((p_end, f_end, h_len))

        elif sec_type == "CORNER_RIGHT_90":
            # 🌀 カービィ64風の滑らかな直角90°右円弧カーブレール
            # 始点(0°) → 中間点(-45°) → 終点(-90°) の2ステップ分割
            radius = self.rng.uniform(12.0, 16.0)
            center = pos + right * radius
            center.z = base_z if environment == "PLAINS" else pos.z

            h_len = radius * 0.276

            # 中間点 (-45°)
            rot45 = mathutils.Matrix.Rotation(-math.radians(45), 3, 'Z')
            p_mid = center + (rot45 @ (-right * radius))
            f_mid = (rot45 @ h_fwd).normalized()
            if environment == "PLAINS":
                p_mid.z = base_z
            sub_points.append((p_mid, f_mid, h_len))

            # 終点 (-90°)
            rot90 = mathutils.Matrix.Rotation(-math.radians(90), 3, 'Z')
            p_end = center + (rot90 @ (-right * radius))
            f_end = (rot90 @ h_fwd).normalized()
            if environment == "PLAINS":
                p_end.z = base_z
            sub_points.append((p_end, f_end, h_len))

        elif sec_type == "CURVE_LEFT":
            # 緩やかな左円弧カーブ（30°〜45°）
            total_angle = self.rng.uniform(30.0, 45.0)
            radius = self.rng.uniform(16.0, 22.0)
            center = pos - right * radius
            center.z = base_z if environment == "PLAINS" else pos.z

            half_angle = total_angle * 0.5
            h_len = radius * math.tan(math.radians(half_angle) * 0.5) * (4.0 / 3.0)

            # 中間点
            rot_half = mathutils.Matrix.Rotation(math.radians(half_angle), 3, 'Z')
            p_mid = center + (rot_half @ (right * radius))
            f_mid = (rot_half @ h_fwd).normalized()
            if environment == "PLAINS":
                p_mid.z = base_z
            sub_points.append((p_mid, f_mid, h_len))

            # 終点
            rot_full = mathutils.Matrix.Rotation(math.radians(total_angle), 3, 'Z')
            p_end = center + (rot_full @ (right * radius))
            f_end = (rot_full @ h_fwd).normalized()
            if environment == "PLAINS":
                p_end.z = base_z
            sub_points.append((p_end, f_end, h_len))

        elif sec_type == "CURVE_RIGHT":
            # 緩やかな右円弧カーブ（30°〜45°）
            total_angle = self.rng.uniform(30.0, 45.0)
            radius = self.rng.uniform(16.0, 22.0)
            center = pos + right * radius
            center.z = base_z if environment == "PLAINS" else pos.z

            half_angle = total_angle * 0.5
            h_len = radius * math.tan(math.radians(half_angle) * 0.5) * (4.0 / 3.0)

            # 中間点
            rot_half = mathutils.Matrix.Rotation(-math.radians(half_angle), 3, 'Z')
            p_mid = center + (rot_half @ (-right * radius))
            f_mid = (rot_half @ h_fwd).normalized()
            if environment == "PLAINS":
                p_mid.z = base_z
            sub_points.append((p_mid, f_mid, h_len))

            # 終点
            rot_full = mathutils.Matrix.Rotation(-math.radians(total_angle), 3, 'Z')
            p_end = center + (rot_full @ (-right * radius))
            f_end = (rot_full @ h_fwd).normalized()
            if environment == "PLAINS":
                p_end.z = base_z
            sub_points.append((p_end, f_end, h_len))

        elif sec_type == "S_CURVE":
            # S字カーブ（左右にうねって元の向きに戻る）
            length = self.rng.uniform(20.0, 26.0)
            sway = self.rng.uniform(4.5, 6.5) * (1.0 if self.rng.random() > 0.5 else -1.0)
            p_mid = pos + h_fwd * (length * 0.5) + right * sway
            p_end = pos + h_fwd * length
            if environment == "PLAINS":
                p_mid.z = base_z
                p_end.z = base_z
            sub_points.append((p_mid, h_fwd, length * 0.2))
            sub_points.append((p_end, h_fwd, length * 0.2))

        elif sec_type == "SLOPE_UP":
            # 上り坂（PLATFORMモード専用）
            length = self.rng.uniform(14.0, 18.0)
            height_gain = self.rng.uniform(3.0, 5.0)
            end_pos = pos + h_fwd * length + up * height_gain
            end_fwd = h_fwd.copy()
            meta["high_platform"] = True
            sub_points.append((end_pos, end_fwd, length * 0.35))

        elif sec_type == "SLOPE_DOWN":
            # 下り坂（PLATFORMモード専用）
            length = self.rng.uniform(14.0, 18.0)
            height_loss = self.rng.uniform(3.0, 4.5)
            end_pos = pos + h_fwd * length - up * height_loss
            end_fwd = h_fwd.copy()
            sub_points.append((end_pos, end_fwd, length * 0.35))

        elif sec_type == "SLOPE_UP_STEP":
            # 階段状に登るアスレチックセクション（PLATFORMモード専用）
            length = self.rng.uniform(16.0, 20.0)
            height_gain = self.rng.uniform(3.5, 5.0)
            end_pos = pos + h_fwd * length + up * height_gain
            end_fwd = h_fwd.copy()
            meta["high_platform"] = True
            meta["is_step"] = True
            sub_points.append((end_pos, end_fwd, length * 0.35))

        else:
            length = 16.0
            end_pos = pos + h_fwd * length
            if environment == "PLAINS":
                end_pos.z = base_z
            end_fwd = h_fwd.copy()
            sub_points.append((end_pos, end_fwd, length * 0.35))

        return sub_points, meta

    def _ensure_plains_ground(self, context, rail_obj, collection, section_infos=None):
        """平原ステージ用の高密度草原地面メッシュ（TERRAIN: terrain_grid）を配置または更新"""
        sampled_points = []
        if rail_obj and rail_obj.type == 'CURVE' and rail_obj.data.splines:
            mat = rail_obj.matrix_world
            for bp in rail_obj.data.splines[0].bezier_points:
                sampled_points.append(mat @ bp.co)

        if not sampled_points:
            return None

        xs = [p.x for p in sampled_points]
        ys = [p.y for p in sampled_points]
        zs = [p.z for p in sampled_points]

        min_x, max_x = min(xs), max(xs)
        min_y, max_y = min(ys), max(ys)
        min_z = min(zs)

        # レール全域を十分に覆うサイズ（余白マージン +80m）
        size_x = max(120.0, (max_x - min_x) + 90.0)
        size_y = max(120.0, (max_y - min_y) + 90.0)
        center_x = (min_x + max_x) * 0.5
        center_y = (min_y + max_y) * 0.5
        center_z = min_z - 0.2  # レールの最低点より少し下

        # 頂点変形・スカルプトが綺麗に効くよう、高密度分割（約1.5m間隔、最大80分割）
        div_x = min(max(int(size_x / 1.5), 32), 80)
        div_y = min(max(int(size_y / 1.5), 32), 80)
        uv_tile = max(4.0, round(size_x / 12.0, 1))

        obj_name = "Terrain_PlainsGround"
        ground_obj = bpy.data.objects.get(obj_name)

        # 高密度グリッドメッシュ生成（UV・マテリアル付与）
        _create_grid = None
        try:
            from .terrain_generator import _create_grid_mesh_data
            _create_grid = _create_grid_mesh_data
        except ImportError:
            try:
                from terrain_generator import _create_grid_mesh_data
                _create_grid = _create_grid_mesh_data
            except ImportError:
                pass

        if _create_grid:
            mesh = _create_grid(
                obj_name + "_Mesh",
                size_x=size_x,
                size_y=size_y,
                div_x=div_x,
                div_y=div_y,
                texture_path="resources/Stagemap/863603.png",
                uv_tile=uv_tile,
            )
        else:
            # フォールバックとして分割グリッドを直接生成
            mesh = bpy.data.meshes.new(obj_name + "_Mesh")
            verts, faces = [], []
            step_x = size_x / div_x
            step_y = size_y / div_y
            start_x = -size_x * 0.5
            start_y = -size_y * 0.5
            for iy in range(div_y + 1):
                for ix in range(div_x + 1):
                    verts.append((start_x + ix * step_x, start_y + iy * step_y, 0.0))
            for iy in range(div_y):
                for ix in range(div_x):
                    i0 = iy * (div_x + 1) + ix
                    faces.append((i0, i0 + 1, i0 + div_x + 2, i0 + div_x + 1))
            mesh.from_pydata(verts, [], faces)
            mesh.update(calc_edges=True)

        if not ground_obj:
            ground_obj = bpy.data.objects.new(obj_name, mesh)
            collection.objects.link(ground_obj)
        else:
            old_mesh = ground_obj.data
            ground_obj.data = mesh
            if old_mesh and old_mesh.users == 0:
                bpy.data.meshes.remove(old_mesh)

        ground_obj.location = mathutils.Vector((center_x, center_y, center_z))
        ground_obj.rotation_euler = (0, 0, 0)
        ground_obj.scale = (1.0, 1.0, 1.0)
        context.view_layer.update()

        # ─── セル属性（cell_types: 0=草, 1=道, 2=穴, 3/4=縁三角）の算出 ───
        mat_rail = rail_obj.matrix_world if rail_obj else mathutils.Matrix.Identity(4)
        gap_segments = []
        if section_infos:
            for s in section_infos:
                if s.get("is_gap", False) and "start_pos" in s and "end_pos" in s:
                    p0 = mat_rail @ s["start_pos"]
                    p1 = mat_rail @ s["end_pos"]
                    gap_segments.append((p0, p1))

        # レールスプラインの細分割サンプリング点列（道幅判定用）
        rail_curve_pts = []
        if rail_obj and rail_obj.type == 'CURVE' and rail_obj.data.splines:
            spline = rail_obj.data.splines[0]
            num_bp = len(spline.bezier_points)
            for i in range(num_bp - 1):
                bp0 = spline.bezier_points[i]
                bp1 = spline.bezier_points[i + 1]
                for step in range(8):
                    t = float(step) / 8.0
                    pt = bp0.co.lerp(bp1.co, t)
                    rail_curve_pts.append(mat_rail @ pt)
            if num_bp > 0:
                rail_curve_pts.append(mat_rail @ spline.bezier_points[-1].co)

        cell_types = []
        start_x = -size_x * 0.5
        start_y = -size_y * 0.5
        step_x = size_x / div_x
        step_y = size_y / div_y

        road_half_width = 2.4 # 道の半幅（約4.8m幅）

        for iy in range(div_y):
            for ix in range(div_x):
                x0 = start_x + ix * step_x
                x1 = start_x + (ix + 1) * step_x
                y0 = start_y + iy * step_y
                y1 = start_y + (iy + 1) * step_y

                # 両方とも穴の外側 -> 通常の床 (道 or 草地)
                wcx = center_x + (x0 + x1) * 0.5
                wcy = center_y + (y0 + y1) * 0.5

                min_rail_dist = 1e9
                for rp in rail_curve_pts:
                    dx = wcx - rp.x
                    dy = wcy - rp.y
                    d_sq = dx * dx + dy * dy
                    if d_sq < min_rail_dist:
                        min_rail_dist = d_sq
                        if min_rail_dist <= road_half_width * road_half_width:
                            break

                if min_rail_dist <= road_half_width * road_half_width:
                    cell_types.append(1) # 1: ROAD（道）
                else:
                    cell_types.append(0) # 0: GROUND（地面・草）

        # ゲームエンジン（C++）動的地形生成用メタデータ
        ground_obj["object_type"] = "TERRAIN"
        ground_obj["file_name"] = "terrain_grid"
        ground_obj["texture"] = "resources/Stagemap/863603.png"
        ground_obj["size_x"] = str(round(size_x, 1))
        ground_obj["size_y"] = str(round(size_y, 1))
        ground_obj["divisions_x"] = str(div_x)
        ground_obj["divisions_y"] = str(div_y)
        ground_obj["uv_tile"] = str(round(uv_tile, 1))
        # export_scene.py 汎用プロパティマップ用 (prop_ プレフィックス)
        ground_obj["prop_size_x"] = str(round(size_x, 1))
        ground_obj["prop_size_y"] = str(round(size_y, 1))
        ground_obj["prop_divisions_x"] = str(div_x)
        ground_obj["prop_divisions_y"] = str(div_y)
        ground_obj["prop_uv_tile"] = str(round(uv_tile, 1))
        ground_obj["prop_grass_texture"] = "resources/Stagemap/863603.png"
        ground_obj["prop_road_texture"] = "resources/grass.png"
        ground_obj["prop_cell_types"] = ",".join(str(ct) for ct in cell_types)

        return ground_obj

    def _carve_pitfall_holes_in_terrain(self, ground_obj, rail_obj, gap_secs, hole_radius=4.2):
        """
        落とし穴セクション（is_gap）の進行ルート直下にある地面グリッドの面を直接削除し、
        完全にメッシュ無しの落下穴（空洞）にする
        """
        if not ground_obj or ground_obj.type != 'MESH' or not gap_secs:
            return

        mesh = ground_obj.data
        mat_world = ground_obj.matrix_world
        mat_rail = rail_obj.matrix_world if rail_obj else mathutils.Matrix.Identity(4)

        hole_points = []
        for sec in gap_secs:
            # ワールド座標での落とし穴始点・終点
            p0 = mat_rail @ sec["start_pos"]
            p1 = mat_rail @ sec["end_pos"]
            length = (p1 - p0).length
            steps = max(int(length / 1.0), 6)
            for step in range(steps + 1):
                t = float(step) / float(steps)
                hole_points.append(p0.lerp(p1, t))

        if not hole_points:
            return

        bm = bmesh.new()
        bm.from_mesh(mesh)
        bm.faces.ensure_lookup_table()

        r_sq = hole_radius * hole_radius
        faces_to_delete = []

        for f in bm.faces:
            # フェースの重心（ワールド座標）
            f_center = mat_world @ f.calc_center_median()
            for hp in hole_points:
                dx = f_center.x - hp.x
                dy = f_center.y - hp.y
                if (dx * dx + dy * dy) < r_sq:
                    faces_to_delete.append(f)
                    break

        if faces_to_delete:
            bmesh.ops.delete(bm, geom=faces_to_delete, context='FACES')
            loose_verts = [v for v in bm.verts if not v.link_faces]
            if loose_verts:
                bmesh.ops.delete(bm, geom=loose_verts, context='VERTS')

        bm.to_mesh(mesh)
        bm.free()
        mesh.update()

    def _trim_unseen_terrain_mesh(self, ground_obj, rail_obj, max_distance=24.0):
        """
        レール（進行ルート）から一定距離（max_distance）以上離れている、
        ゲーム中カメラに映らない不要な遠方メッシュの面を削除して最適化する
        """
        if not ground_obj or ground_obj.type != 'MESH' or not rail_obj:
            return

        # レール上のサンプリング点列を取得
        sampled_pts = []
        if rail_obj.type == 'CURVE' and rail_obj.data.splines:
            mat_rail = rail_obj.matrix_world
            spline = rail_obj.data.splines[0]
            num_bp = len(spline.bezier_points)
            for i in range(num_bp - 1):
                p0 = spline.bezier_points[i]
                p1 = spline.bezier_points[i + 1]
                for step in range(8):
                    t = float(step) / 8.0
                    pt = p0.co.lerp(p1.co, t)
                    sampled_pts.append(mat_rail @ pt)
            if num_bp > 0:
                sampled_pts.append(mat_rail @ spline.bezier_points[-1].co)

        if not sampled_pts:
            return

        mesh = ground_obj.data
        mat_world = ground_obj.matrix_world

        bm = bmesh.new()
        bm.from_mesh(mesh)
        bm.faces.ensure_lookup_table()

        max_dist_sq = max_distance * max_distance
        faces_to_delete = []

        for f in bm.faces:
            f_center = mat_world @ f.calc_center_median()
            # レール点列への最短水平距離（XY）を判定
            min_d_sq = 1e9
            for rp in sampled_pts:
                dx = f_center.x - rp.x
                dy = f_center.y - rp.y
                d_sq = dx * dx + dy * dy
                if d_sq < min_d_sq:
                    min_d_sq = d_sq
                    if min_d_sq <= max_dist_sq:
                        break  # 十分近いので保持

            if min_d_sq > max_dist_sq:
                faces_to_delete.append(f)

        if faces_to_delete:
            bmesh.ops.delete(bm, geom=faces_to_delete, context='FACES')
            loose_verts = [v for v in bm.verts if not v.link_faces]
            if loose_verts:
                bmesh.ops.delete(bm, geom=loose_verts, context='VERTS')

        bm.to_mesh(mesh)
        bm.free()
        mesh.update()

    def _populate_stage(self, context, rail_obj, section_infos, base_seg_index, environment, spawn_blocks, enemy_prob, gap_prob, collection):
        """レールセクションに合わせて足場ブロックと敵を生成配置"""
        blocks = []
        enemies = []

        block_spacing = 3.0   # ブロック間隔（約3m）
        block_width = 4.0     # 道の横幅

        # 平原ステージかアスレチックかで足場スタイルを切り替え
        if environment == "PLAINS":
            block_thickness = 0.25 # 草原の小道プレート（薄型）
            block_texture = "resources/Stagemap/863603.png"
        else:
            block_thickness = 1.0  # アスレチック用直方体ブロック
            block_texture = "resources/grass.png"

        # 敵プレビューモデルの取得
        preview_mesh = None
        try:
            from .import_scene import get_or_load_preview_mesh
            preview_mesh = get_or_load_preview_mesh("ENEMY")
        except Exception:
            try:
                import import_scene
                preview_mesh = import_scene.get_or_load_preview_mesh("ENEMY")
            except Exception:
                pass

        for s_idx, sec in enumerate(section_infos):
            p0 = sec["start_pos"]
            p1 = sec["end_pos"]
            fwd0 = sec["start_fwd"]
            fwd1 = sec["end_fwd"]
            sec_len = (p1 - p0).length
            num_steps = max(int(sec_len / block_spacing), 2)
            is_gap_section = sec.get("is_gap", False)

            for step in range(num_steps):
                t = float(step) / float(num_steps)
                # エルミート補間またはベジェで中間点と向きを概算
                interp_pos = p0.lerp(p1, t)
                interp_fwd = fwd0.lerp(fwd1, t).normalized()

                # 水平回転角（Yaw）
                yaw = math.atan2(interp_fwd.y, interp_fwd.x) - (math.pi / 2.0)

                # ─── 足場ブロック生成 ───
                is_step_section = sec.get("is_step", False)
                # 穴セクションの場合、中央のブロックを抜いてジャンプ穴にする
                is_hole = is_gap_section and (0.3 <= t <= 0.7)

                # PLATFORM モードのみ空中ブロックを配置（PLAINSモードでは平原地面を活かすため直方体ブロックは置かない）
                if environment == "PLATFORM" and spawn_blocks and not is_hole:
                    # 階段セクションの場合、ステップ段差を強調して階段状に配置
                    blk_z_offset = -block_thickness * 0.5
                    if is_step_section:
                        blk_z_offset += (step * 0.25)

                    blk = self._create_stage_block(
                        f"StageBlock_S{s_idx}_{step}",
                        interp_pos + mathutils.Vector((0, 0, blk_z_offset)),
                        (block_width, block_spacing * 1.05, block_thickness),
                        yaw,
                        block_texture,
                        collection
                    )
                    blocks.append(blk)

                    # 稀に頭上にすり抜け足場（ボーナス足場）を配置
                    if self.rng.random() < 0.25 and not is_step_section:
                        oneway_prefab = _BLOCK_PREFABS["oneway_platform"]
                        oneway_blk = self._create_stage_block(
                            f"OneWayPlatform_S{s_idx}_{step}",
                            interp_pos + mathutils.Vector((0, 0, 2.5)),
                            oneway_prefab["size"],
                            yaw,
                            oneway_prefab["texture"],
                            collection,
                            is_oneway=True
                        )
                        blocks.append(oneway_blk)

                # 穴セクションで真ん中に浮島や浮遊足場を作る演出 (PLATFORMのみ)
                if environment == "PLATFORM" and spawn_blocks and is_hole and step == num_steps // 2:
                    if self.rng.random() < 0.4:
                        # 浮島（広めの島）
                        island_prefab = _BLOCK_PREFABS["floating_island"]
                        island_blk = self._create_stage_block(
                            f"IslandBlock_S{s_idx}_{step}",
                            interp_pos + mathutils.Vector((0, 0, -island_prefab["size"][2] * 0.5)),
                            island_prefab["size"],
                            yaw,
                            island_prefab["texture"],
                            collection,
                            is_oneway=False
                        )
                        blocks.append(island_blk)
                    elif self.rng.random() < 0.6:
                        # すり抜け足場または通常浮遊足場
                        use_oneway = (self.rng.random() < 0.5)
                        prefab_key = "oneway_platform" if use_oneway else "standard"
                        chosen_p = _BLOCK_PREFABS[prefab_key]
                        float_blk = self._create_stage_block(
                            f"FloatBlock_S{s_idx}_{step}",
                            interp_pos + mathutils.Vector((0, 0, -chosen_p["size"][2] * 0.5)),
                            (chosen_p["size"][0] * 0.8, chosen_p["size"][1] * 0.8, chosen_p["size"][2]),
                            yaw,
                            chosen_p["texture"],
                            collection,
                            is_oneway=chosen_p["is_oneway"]
                        )
                        blocks.append(float_blk)

                # ─── 敵の配置 ───
                # スタート地点（最初の2セクション s_idx < 2、および始点から25m以内）は完全な安全地帯として絶対に敵を湧かせない
                should_spawn_enemy = False
                enemy_type = "Normal"

                start_co = section_infos[0]["start_pos"]
                dist_from_start = (interp_pos - start_co).length

                if s_idx >= 2 and dist_from_start >= 25.0:
                    if is_gap_section and step == 1:
                        # 穴の手前に構える敵
                        if self.rng.random() < enemy_prob * 1.2:
                            should_spawn_enemy = True
                            enemy_type = "Bound" if self.rng.random() < 0.5 else "Normal"
                    elif not is_hole and step == num_steps // 2:
                        # 区間の中央に配置
                        if self.rng.random() < enemy_prob:
                            should_spawn_enemy = True
                            enemy_type = "Normal" if self.rng.random() < 0.7 else "Bound"

                if should_spawn_enemy:
                    # レール上のグローバル進行度 t を正確に計算
                    global_t = float(base_seg_index + s_idx) + t
                    enemy_pos = interp_pos + mathutils.Vector((0, 0, 0.5))
                    rail_pos = [round(global_t, 3), 0.5]

                    enemy_obj = self._create_enemy(
                        f"Enemy_AI_S{s_idx}_{step}",
                        enemy_pos,
                        yaw,
                        enemy_type,
                        rail_pos,
                        preview_mesh,
                        collection
                    )

                    # レール位置へ正確にスナップ配置
                    try:
                        from .rail_snap import apply_rail_position_to_object
                        apply_rail_position_to_object(enemy_obj, rail_obj)
                    except Exception:
                        try:
                            import rail_snap
                            rail_snap.apply_rail_position_to_object(enemy_obj, rail_obj)
                        except Exception:
                            pass

                    enemies.append(enemy_obj)

        return blocks, enemies

    def _create_stage_block(self, name, pos, size, yaw, texture_path, collection, is_oneway=False):
        """直方体または小道プレートの足場ブロック（BLOCK）を生成"""
        mesh = bpy.data.meshes.new(name)
        obj = bpy.data.objects.new(name, mesh)
        collection.objects.link(obj)

        # キューブ形状を直接頂点で生成（高速かつ確実）
        hw = size[0] * 0.5
        hl = size[1] * 0.5
        hh = size[2] * 0.5
        verts = [
            (-hw, -hl, -hh), (hw, -hl, -hh), (hw, hl, -hh), (-hw, hl, -hh),
            (-hw, -hl, hh),  (hw, -hl, hh),  (hw, hl, hh),  (-hw, hl, hh),
        ]
        faces = [
            (0, 1, 2, 3), (4, 7, 6, 5),
            (0, 4, 5, 1), (1, 5, 6, 2),
            (2, 6, 7, 3), (3, 7, 4, 0),
        ]
        mesh.from_pydata(verts, [], faces)
        mesh.update()

        obj.location = pos
        obj.rotation_euler = (0, 0, yaw)

        # ゲームエンジン用メタデータ
        obj["object_type"] = "BLOCK"
        obj["file_name"] = "box"
        obj["texture"] = texture_path
        if is_oneway:
            obj["prop_is_oneway"] = "true"

        return obj

    def _create_enemy(self, name, pos, yaw, enemy_type, rail_pos, preview_mesh, collection):
        """エネミー（ENEMY）オブジェクトを生成"""
        if preview_mesh:
            obj = bpy.data.objects.new(name, preview_mesh)
        else:
            obj = bpy.data.objects.new(name, None)
            obj.empty_display_type = 'ARROWS'

        collection.objects.link(obj)
        obj.location = pos
        obj.rotation_euler = (0, 0, yaw)

        # ゲームエンジン用メタデータ
        obj["object_type"] = "ENEMY"
        obj["enemy_type"] = enemy_type
        obj["file_name"] = "taru.obj"
        obj["model_dir"] = "resources"
        obj["rail_pos"] = list(rail_pos)

        return obj

    def _handle_player_spawn_position(self, context, rail_obj, collection):
        """既存のPlayerSpawnオブジェクトをレール始点に移動、または新規作成"""
        player_obj = None
        for obj in bpy.data.objects:
            if obj.get("object_type") == "PLAYER_SPAWN" or obj.name.lower() in ("playerspawn", "player_spawn", "player"):
                player_obj = obj
                break

        # レール始点の座標と向きを取得
        start_pos = mathutils.Vector((0.0, 0.0, 0.5))
        yaw = 0.0
        if rail_obj and rail_obj.type == 'CURVE' and rail_obj.data.splines:
            bp0 = rail_obj.data.splines[0].bezier_points[0]
            mat = rail_obj.matrix_world
            start_pos = mat @ bp0.co + mathutils.Vector((0.0, 0.0, 0.5))
            fwd = bp0.handle_right - bp0.co
            if fwd.length > 1e-4:
                yaw = math.atan2(fwd.y, fwd.x) - (math.pi / 2.0)

        if player_obj:
            player_obj.location = start_pos
            player_obj.rotation_euler = (0, 0, yaw)
        else:
            # 新規プレイヤー開始位置作成
            preview_mesh = None
            try:
                from .import_scene import get_or_load_preview_mesh
                preview_mesh = get_or_load_preview_mesh("PLAYER_SPAWN")
            except Exception:
                try:
                    import import_scene
                    preview_mesh = import_scene.get_or_load_preview_mesh("PLAYER_SPAWN")
                except Exception:
                    pass

            if preview_mesh:
                player_obj = bpy.data.objects.new("PlayerSpawn", preview_mesh)
            else:
                player_obj = bpy.data.objects.new("PlayerSpawn", None)
                player_obj.empty_display_type = 'SINGLE_ARROW'

            collection.objects.link(player_obj)
            player_obj.location = start_pos
            player_obj.rotation_euler = (0, 0, yaw)
            player_obj["object_type"] = "PLAYER_SPAWN"
            player_obj["file_name"] = "player.obj"
            player_obj["model_dir"] = "resources/player"
            player_obj["texture"] = "resources/player/player.png"

        # レール始点 (t=0.0, 高さオフセット=0.5) にスナップ
        player_obj["rail_pos"] = [0.0, 0.5]
        if rail_obj:
            try:
                from .rail_snap import apply_rail_position_to_object
                apply_rail_position_to_object(player_obj, rail_obj)
            except Exception:
                try:
                    import rail_snap
                    rail_snap.apply_rail_position_to_object(player_obj, rail_obj)
                except Exception:
                    pass

        return player_obj

    def _handle_goal_position(self, context, rail_obj, end_pos, end_fwd, collection):
        """既存のGoalオブジェクトを終端に移動、または新規作成"""
        goal_obj = None
        for obj in bpy.data.objects:
            if obj.get("object_type") == "GOAL" or obj.name.lower() == "goal":
                goal_obj = obj
                break

        yaw = math.atan2(end_fwd.y, end_fwd.x) - (math.pi / 2.0)
        goal_pos = end_pos + mathutils.Vector((0, 0, 1.0))

        if goal_obj:
            goal_obj.location = goal_pos
            goal_obj.rotation_euler = (0, 0, yaw)
        else:
            # 新規ゴール作成
            preview_mesh = None
            try:
                from .import_scene import get_or_load_preview_mesh
                preview_mesh = get_or_load_preview_mesh("GOAL")
            except Exception:
                pass

            if preview_mesh:
                goal_obj = bpy.data.objects.new("Goal", preview_mesh)
            else:
                goal_obj = bpy.data.objects.new("Goal", None)
                goal_obj.empty_display_type = 'SPHERE'

            collection.objects.link(goal_obj)
            goal_obj.location = goal_pos
            goal_obj.rotation_euler = (0, 0, yaw)
            goal_obj["object_type"] = "GOAL"
            goal_obj["file_name"] = "goal.obj"
            goal_obj["model_dir"] = "resources"

        # ゴールにも最新終端の rail_pos を設定してスナップ
        if rail_obj and rail_obj.type == 'CURVE' and rail_obj.data.splines:
            last_t = max(0.0, float(len(rail_obj.data.splines[0].bezier_points) - 1))
            goal_obj["rail_pos"] = [round(last_t, 3), 0.5]
            try:
                from .rail_snap import apply_rail_position_to_object
                apply_rail_position_to_object(goal_obj, rail_obj)
            except Exception:
                try:
                    import rail_snap
                    rail_snap.apply_rail_position_to_object(goal_obj, rail_obj)
                except Exception:
                    pass

        return goal_obj


# =========================================================================
# 3. Blender オペレーター
# =========================================================================

class MYADDON_OT_ai_generate_stage(bpy.types.Operator):
    bl_idname = "myaddon.ai_generate_stage"
    bl_label = "🌟 AIステージ自動生成・伸長"
    bl_description = "カービィ64風のコース（レール・足場・敵・カメラレール）を自動生成・伸長します"
    bl_options = {"REGISTER", "UNDO"}

    environment: bpy.props.EnumProperty(
        name="ステージ環境 (地形)",
        description="ステージの地面・足場の種類",
        items=[
            ('PLAINS', "🌿 緑の平原 (Green Plains)", "広大な草原地面（terrain_grid）となだらかな草道・丘陵コース"),
            ('PLATFORM', "🧱 浮遊アスレチック (Platforms)", "空中に直方体ブロックが並ぶアスレチックコース"),
        ],
        default='PLAINS'
    )

    camera_side: bpy.props.EnumProperty(
        name="カメラ視点位置",
        description="プレイヤーの進行方向に対するカメラレールの配置位置",
        items=[
            ('RIGHT', "右側 (Right Side / 2.5D標準)", "進行方向の右手側から撮影（プレイヤーの右側）"),
            ('LEFT', "左側 (Left Side / 反転)", "進行方向の左手側から撮影"),
        ],
        default='RIGHT'
    )

    section_count: bpy.props.IntProperty(
        name="伸長セクション数",
        description="追加するコースの区間数（1区間あたり約12〜16m）",
        default=12,
        min=3,
        max=50
    )

    theme: bpy.props.EnumProperty(
        name="コーススタイル",
        description="生成するコースの難易度や特徴",
        items=[
            ('BALANCED', "バランス (Balanced)", "直線・カーブ・坂・敵がバランスよく配置された標準ステージ"),
            ('BEGINNER', "初心者向け (Beginner)", "直線多めで穴がなく、ゆったり遊べる平坦コース"),
            ('ROLLER_COASTER', "ジェットコースター (3D Action)", "直角コーナーやS字カーブ、起伏に富んだダイナミックコース"),
            ('CHALLENGE', "アスレチック (Challenge)", "穴や浮遊足場、敵の配置が多いテクニカルコース"),
        ],
        default='BALANCED'
    )

    curve_intensity: bpy.props.FloatProperty(
        name="カーブの激しさ",
        description="曲がり角や直角コーナーの発生しやすさ",
        default=0.5,
        min=0.0,
        max=1.0,
        subtype='FACTOR'
    )

    slope_intensity: bpy.props.FloatProperty(
        name="高低差 (坂道)",
        description="上り坂・下り坂の発生しやすさ",
        default=0.4,
        min=0.0,
        max=1.0,
        subtype='FACTOR'
    )

    gap_density: bpy.props.FloatProperty(
        name="落とし穴 (ジャンプ穴) の頻度",
        description="飛び越える必要がある落とし穴（落下ゾーン・クレバス）の発生率",
        default=0.3,
        min=0.0,
        max=1.0,
        subtype='FACTOR'
    )

    enemy_density: bpy.props.FloatProperty(
        name="敵の出現頻度",
        description="通常敵やバウンド敵の出現頻度",
        default=0.5,
        min=0.0,
        max=1.0,
        subtype='FACTOR'
    )

    spawn_blocks: bpy.props.BoolProperty(
        name="足場ブロックを自動配置",
        description="レールに沿って自動で接地用足場ブロックを敷き詰める",
        default=True
    )

    update_camera: bpy.props.BoolProperty(
        name="カメラレールを自動同期",
        description="ステージ伸長に合わせてカメラレールも追従更新する",
        default=True
    )

    move_goal: bpy.props.BoolProperty(
        name="ゴールを新終端へ移動",
        description="Goalオブジェクトを新しく伸ばしたレールの終端へ自動移動させる",
        default=True
    )

    deform_terrain_auto: bpy.props.BoolProperty(
        name="地面をレール沿いに自動変形",
        description="生成と同時にレールのカーブや高低差に合わせて地面メッシュを自動造形します",
        default=True
    )

    reset_existing: bpy.props.BoolProperty(
        name="既存コースをリセットして新規作成",
        description="既存のステージレールや生成オブジェクトを初期化し、新しいコースを最初から生成します（オフの場合は現在の終端から伸長）",
        default=True
    )

    random_seed: bpy.props.IntProperty(
        name="ランダムシード (0で毎回完全ランダム)",
        description="生成パターンを決めるシード値（0の場合はミリ秒タイムスタンプから毎回異なるランダムコースを生成）",
        default=0
    )

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self, width=380)

    def draw(self, context):
        layout = self.layout

        box_mode = layout.box()
        box_mode.label(text="🔄 生成モード:", icon='FILE_REFRESH')
        box_mode.prop(self, "reset_existing")

        box_env = layout.box()
        box_env.label(text="🌍 ステージ環境 & カメラ:", icon='WORLD')
        box_env.prop(self, "environment")
        if self.environment == 'PLAINS':
            box_env.prop(self, "deform_terrain_auto")
        box_env.prop(self, "camera_side")

        box_params = layout.box()
        box_params.label(text="⚙️ コース生成設定:", icon='PREFERENCES')
        box_params.prop(self, "section_count")
        box_params.prop(self, "theme")
        box_params.prop(self, "curve_intensity")
        box_params.prop(self, "slope_intensity")
        box_params.prop(self, "gap_density")
        box_params.prop(self, "enemy_density")

        box_opt = layout.box()
        box_opt.label(text="🔧 その他オプション:", icon='TOOL_SETTINGS')
        if self.environment == 'PLATFORM':
            box_opt.prop(self, "spawn_blocks")
        box_opt.prop(self, "update_camera")
        box_opt.prop(self, "move_goal")
        box_opt.prop(self, "random_seed")

    def execute(self, context):
        # シードが0の場合はミリ秒単位で完全に異なる乱数シードを生成
        if self.random_seed != 0:
            seed_val = self.random_seed
        else:
            seed_val = int(time.time() * 1000) % 2147483647
            if seed_val == 0:
                seed_val = random.randint(1, 9999999)

        generator = StageAIGenerator(seed=seed_val)

        settings = {
            "section_count": self.section_count,
            "theme": self.theme,
            "environment": self.environment,
            "camera_side": self.camera_side,
            "curve_intensity": self.curve_intensity,
            "slope_intensity": self.slope_intensity,
            "gap_density": self.gap_density,
            "enemy_density": self.enemy_density,
            "spawn_blocks": self.spawn_blocks,
            "update_camera": self.update_camera,
            "move_goal": self.move_goal,
            "deform_terrain_auto": self.deform_terrain_auto,
            "reset_existing": self.reset_existing,
        }

        try:
            num_secs, num_objs = generator.generate(context, settings)
            mode_text = "新規生成" if self.reset_existing else "伸長"
            self.report(
                {"INFO"},
                f"✅ ステージを{mode_text}しました: {num_secs}セクション, {num_objs}個のオブジェクトを配置 (Seed: {seed_val})"
            )
            return {"FINISHED"}
        except Exception as e:
            self.report({"ERROR"}, f"ステージ自動生成エラー: {e}")
            import traceback
            traceback.print_exc()
            return {"CANCELLED"}
