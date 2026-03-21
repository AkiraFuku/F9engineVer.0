#include "ThreadManager.h"

#include "Camera.h"
#include "DrawFunction.h"

#include <cassert>

/// <summary>
/// 初期化
/// </summary>
/// <param name="maxThreads">最大描画本数</param>
/// <param name="nodesPerThread">1本あたりのノード数</param>
void ThreadManager::Initialize(int maxThreads, int nodesPerThread,
    Camera* camera) {
    // カメラを受け取る
    assert(camera);
    camera_ = camera;

    // 最大描画本数を設定
    maxThreads_ = maxThreads;
    // 1本当たりのノード数を設定
    nodesPerThread_ = nodesPerThread;

    // メモリの再確保を防ぐため、あらかじめ容量を確保
    physicsList_.reserve(maxThreads_);

    renderer_ = std::make_unique<ThreadRenderer>();
    // 太さや分割数はひとまず固定値（必要なら引数化してください）
    renderer_->Initialize(maxThreads, nodesPerThread, 0.02f, 6);
}

/// <summary>
/// 更新
/// </summary>
void ThreadManager::Update() {
    // 描画用のノード配列を収集
    std::vector<std::vector<PhysicsNode>> allNodes;
    allNodes.reserve(physicsList_.size());

    // 各糸の物理演算を更新
    for (auto& physics : physicsList_) {
        physics->Update();
        allNodes.push_back(physics->GetNodes());
    }

    // 収集した全ノードをRendererに渡し、一気に頂点バッファを書き換える
    renderer_->Update(allNodes, camera_);
}

/// <summary>
/// 描画
/// </summary>
void ThreadManager::Draw() { renderer_->Draw(); }

void ThreadManager::AddThread(const Vector3& startPos, const Vector3& endPos) {
    // 最大数を超えていたら追加しない（または一番古いものを消す等）
    if (physicsList_.size() >= maxThreads_) {
        return;
    }

    auto physics = std::make_unique<ThreadPhysics>();
    physics->Initialize(startPos, endPos, nodesPerThread_);

    physicsList_.push_back(std::move(physics));
}

void ThreadManager::ClearThreads() { physicsList_.clear(); }

// ThreadManager.cpp

bool ThreadManager::IsOnThread(const Vector3& pos, float radius) const {
    float radiusSq = radius * radius;

    // プレイヤーのY座標を0にした判定用ベクトルを作る
    Vector3 flatPos = {pos.x, 0.0f, pos.z};

    for (const auto& physics : physicsList_) {
        const auto& nodes = physics->GetNodes();
        if (nodes.size() < 2) continue;

        for (size_t i = 0; i < nodes.size() - 1; ++i) {
            // ノードのY座標も0にして、上から見下ろした2D（XZ平面）上で距離を測る
            Vector3 flatA = {nodes[i].currentPos.x, 0.0f, nodes[i].currentPos.z};
            Vector3 flatB = {nodes[i + 1].currentPos.x, 0.0f, nodes[i + 1].currentPos.z};

            float distSq = DistanceSqPointToSegment(flatPos, flatA, flatB);

            if (distSq <= radiusSq) {
                return true;
            }
        }
    }
    return false;
}

/// <summary>
/// プレイヤーの重みで糸をたわませる
/// </summary>
/// <param name="pos">プレイヤーの座標</param>
/// <param name="radius">影響範囲</param>
/// <param name="weight">重みの強さ</param>
void ThreadManager::ApplyPlayerWeight(const Vector3& pos, float radius, float weight) {
    // 下向きの力ベクトルを作成
    Vector3 force = {0.0f, -weight, 0.0f};

    for (auto& physics : physicsList_) {
        physics->ApplyForce(pos, radius, force);
    }
}

/// <summary>
/// プレイヤー座標から一番近い糸の高さを取得する
/// </summary>
/// <param name="pos">プレイヤーの座標</param>
/// <param name="radius">判定半径</param>
/// <param name="outY">取得したY座標を入れる変数</param>
/// <returns>糸の上にいるならtrue</returns>
bool ThreadManager::GetThreadHeight(const Vector3& pos, float radius, float& outY) const {
    float radiusSq = radius * radius;
    Vector3 flatPos = {pos.x, 0.0f, pos.z};

    float closestDistSq = radiusSq + 1.0f;
    bool found = false;
    float bestY = 0.0f;

    for (const auto& physics : physicsList_) {
        const auto& nodes = physics->GetNodes();
        if (nodes.size() < 2) continue;

        for (size_t i = 0; i < nodes.size() - 1; ++i) {
            Vector3 flatA = {nodes[i].currentPos.x, 0.0f, nodes[i].currentPos.z};
            Vector3 flatB = {nodes[i + 1].currentPos.x, 0.0f, nodes[i + 1].currentPos.z};

            // 線分ABと、Aからプレイヤーへのベクトル
            Vector3 ab = flatB - flatA;
            Vector3 ap = flatPos - flatA;

            // プレイヤーが線分ABのどの辺り（割合）にいるかを計算 (0.0 ～ 1.0)
            float ab2 = ab.x * ab.x + ab.z * ab.z;
            float t = 0.0f;
            if (ab2 > 0.0f) {
                t = (ap.x * ab.x + ap.z * ab.z) / ab2;
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
            }

            // 線分上の最も近い点（XZ平面）を求めて距離を測る
            Vector3 closestFlat = {flatA.x + ab.x * t, 0.0f, flatA.z + ab.z * t};
            float dx = flatPos.x - closestFlat.x;
            float dz = flatPos.z - closestFlat.z;
            float distSq = dx * dx + dz * dz;

            if (distSq <= radiusSq) {
                // 複数の糸が重なっていた場合、一番近いものを優先する
                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    found = true;

                    // ここがキモ！実際のノードのY座標を使って、たわんだ高さを補間計算する
                    float yA = nodes[i].currentPos.y;
                    float yB = nodes[i + 1].currentPos.y;
                    bestY = yA + (yB - yA) * t;
                }
            }
        }
    }

    if (found) {
        outY = bestY;
        return true;
    }
    return false;
}