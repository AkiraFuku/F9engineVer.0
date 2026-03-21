#include "ThreadPhysics.h"

#include "MathFunction.h"

#include <cmath>
#include <algorithm>

/// <summary>
/// 糸の初期化
/// </summary>
/// <param name="startPos">糸の始点</param>
/// <param name="endPos">糸の終点</param>
/// <param name="nodeCount">節の数</param>
void ThreadPhysics::Initialize(const Vector3& startPos, const Vector3& endPos, int nodeCount) {
    // 線を形成するには最低2つのノード（始点と終点）が必要
    if (nodeCount < 2)
    {
        return;
    }

    nodes_.clear();
    // 実行時のメモリ再割り当てを防ぎ、初期化を高速化
    nodes_.reserve(nodeCount);

    Vector3 difference = endPos - startPos;
    float totalLength = Length(difference);

    // ノード間の理想的な距離（自然長）を算出
    segmentLength_ = totalLength / static_cast<float>(nodeCount - 1);

    for (int i = 0; i < nodeCount; ++i) {
        // 0.0 (始点) ～ 1.0 (終点) の補間係数
        float t = static_cast<float>(i) / static_cast<float>(nodeCount - 1);

        // 始点から終点に向かって一直線上にノードを配置
        Vector3 pos = startPos + (difference * t);

        PhysicsNode node;
        node.currentPos = pos;
        node.previousPos = pos;

        // 両端（始点と終点）は壁に固定（質量0）、中間のノードは自由に動く（質量1）
        node.mass = (i == 0 || i == nodeCount - 1) ? 0.0f : 1.0f;

        nodes_.push_back(node);
    }
}

/// <summary>
/// 更新
/// </summary>
void ThreadPhysics::Update() {
    // 慣性と重力による移動処理
    Integrate();
    // 糸の長さが保たれるように位置を補正
    SolveConstraints();
}

/// <summary>
/// Verlet積分による各ノードの位置更新
/// </summary>
void ThreadPhysics::Integrate() {
    for (auto& node : nodes_) {
        // 固定ノード（質量0以下）は動かさない
        if (node.mass <= 0.0f)
        {
            continue;
        }

        // 過去の位置との差分から「現在の速度」を擬似的に算出（Verlet積分の特徴）
        Vector3 velocity = node.currentPos - node.previousPos;

        // 現在の座標を「過去の座標」として記録し更新に備える
        node.previousPos = node.currentPos;

        // 慣性（速度 × 空気抵抗）と重力を足し合わせて、次の予測位置へ進める
        node.currentPos += (velocity * damping_) + gravity_;
    }
}

/// <summary>
/// ノード間の距離制約の解決（長さの維持）
/// </summary>
void ThreadPhysics::SolveConstraints() {
    if (nodes_.size() < 2)
    {
        return;
    }

    // 複数回反復（イテレーション）することで、伸びにくく硬い糸をシミュレート
    for (int i = 0; i < iterations_; ++i) {
        for (size_t j = 0; j < nodes_.size() - 1; ++j) {
            PhysicsNode& n1 = nodes_[j];
            PhysicsNode& n2 = nodes_[j + 1];

            // 両方とも固定ノードなら計算不要
            if (n1.mass <= 0.0f && n2.mass <= 0.0f)
            {
                continue;
            }

            Vector3 diff = n1.currentPos - n2.currentPos;
            float currentDist = Length(diff);

            // 距離が近すぎる場合のゼロ除算エラーと計算の爆発（NaN）を防止
            if (currentDist <= 1e-6f)
            {
                continue;
            }

            // 理想の長さに対して、現在どれだけ伸び縮みしているかの割合
            float difference = (segmentLength_ - currentDist) / currentDist;

            // 動くノード（質量>0）にのみ補正を適用するための重み付け
            float w1 = (n1.mass > 0.0f) ? 1.0f : 0.0f;
            float w2 = (n2.mass > 0.0f) ? 1.0f : 0.0f;
            float weightSum = w1 + w2;

            // 2点の距離を理想の長さに戻すための補正ベクトル
            Vector3 correction = diff * (difference / weightSum);

            // 重みに応じて互いに押し引きし、理想の距離に近づける
            if (n1.mass > 0.0f) n1.currentPos += correction * w1;
            if (n2.mass > 0.0f) n2.currentPos -= correction * w2;
        }
    }
}

/// <summary>
/// 指定範囲内のノードに力を加える
/// </summary>
/// <param name="pos">力を加える中心（プレイヤーや敵の座標）</param>
/// <param name="radius">影響範囲の半径</param>
/// <param name="force">加える力のベクトル（重み）</param>
void ThreadPhysics::ApplyForce(const Vector3& pos, float radius, const Vector3& force) {
    if (radius <= 0.0f)
    {
        return;
    }

    // 毎回の平方根計算（処理負荷）を省くため、距離の二乗同士で比較する
    float radiusSq = radius * radius;

    for (auto& node : nodes_) {
        if (node.mass <= 0.0f)
        {
            continue;
        }

        Vector3 difference = node.currentPos - pos;
        // ※ MathFunctionに LengthSq() があれば、そちらへの置き換えを推奨
        float distSq = difference.x * difference.x + difference.y * difference.y + difference.z * difference.z;

        // ノードが影響範囲（球体）の中に入っているか判定
        if (distSq <= radiusSq) {
            float dist = std::sqrt(distSq);

            // 中心に近いほど力強く（1.0）、端に行くほど弱く（0.0）なる減衰率
            float falloff = 1.0f - (dist / radius);

            // 位置を直接ずらす。このズレは次フレームのIntegrate()で速度として解釈される
            node.currentPos += force * falloff;
        }
    }
}