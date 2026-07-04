#include "Efect.h"
#include <random>
#include <functional>
#include <cmath>

// =================================================================
// 1. 円柱（Cylinder）のエフェクト処理
// =================================================================

// 初期化：最初は縦に長い鋭い柱として生成する
ParticleManager::ParticleEmitterFunc CylinderInitFunc = [](const Vector3& emitterPosition, std::mt19937& randomEngine) -> ParticleManager::Particle {
    ParticleManager::Particle particle;
    
    // 最初は縦（Y）に長く、直径（XZ）は細めにする
    particle.transform.scale = { 0.5f, 4.0f, 0.5f };
    particle.transform.rotate = { 0.0f, 0.0f, 0.0f }; // 垂直に立てる
    particle.transform.translate = emitterPosition;
    particle.velocity = { 0.0f, 0.0f, 0.0f };

    // 不透明度を高めに設定（加算合成で強く光らせるため）
    particle.color = { 1.0f, 1.0f, 1.0f, 1.0f }; 

    // 円柱の寿命は一瞬（0.15秒など、超高速で潰すため短く設定）
    particle.lifeTime = 0.15f; 
    particle.currentTime = 0.0f;

    // UVスクロールの初期設定（必要に応じて）
    particle.uvTransform.scale = { 1.0f, 1.0f };
    particle.uvTransform.rotate = { 0.0f };
    particle.uvTransform.offset = { 0.0f, 0.0f };

    return particle;
};

// 更新：上下から超高速で押しつぶされるアニメーション
ParticleManager::ParticleUpdateFunc CylinderUpdateFunc = [](ParticleManager::Particle& particle, float deltaTime) {
    particle.currentTime += deltaTime;
    float t = particle.currentTime / particle.lifeTime; // 0.0f ～ 1.0f の進捗率

    // イージング（後半に向かって加速して地面に叩きつけられるように潰す）
    float easeInQuad = t * t;

    // Yスケールを 4.0f から 0.01f（ほぼペラペラ）へ超高速で潰す
    particle.transform.scale.y = (1.0f - easeInQuad) * 4.0f + 0.01f;

    // 潰れる反動で、横幅（XZ）を少しだけ広げる
    float sideScale = 0.5f + easeInQuad * 1.5f;
    particle.transform.scale.x = sideScale;
    particle.transform.scale.z = sideScale;

    // UVを縦に超高速スクロールさせてエネルギーの激しさを表現
    particle.uvTransform.offset.y -= deltaTime * 8.0f;

    // 地面に激突して消える直前にフェードアウト
    if (t > 0.8f) {
        particle.color.w = (1.0f - t) / 0.2f;
    }
};


// =================================================================
// 2. リング（Ring）のエフェクト処理（複数枚を時間差で広げる用）
// =================================================================

// 初期化：最初は極小のサイズからスタート
ParticleManager::ParticleEmitterFunc RingInitFunc = [](const Vector3& emitterPosition, std::mt19937& randomEngine) -> ParticleManager::Particle {
    ParticleManager::Particle particle;
    
    // 最初はサイズほぼ0
    particle.transform.scale = { 0.01f, 0.01f, 0.01f };
    // 地面に対して水平にするため、X軸を90度（π/2）回転させる
    particle.transform.rotate = { 1.57079f, 0.0f, 0.0f }; 
    particle.transform.translate = emitterPosition;
    particle.velocity = { 0.0f, 0.0f, 0.0f };

    // 発光色（白・シアン・紫などお好みで。ここでは白ベース）
    particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    // リングは余韻を残すため、円柱より少し長めの寿命（0.4秒など）
    particle.lifeTime = 0.4f; 
    particle.currentTime = 0.0f;

    return particle;
};

// 更新：ドンッ！と最高速で広がり、ジワッと減速しながら消える
ParticleManager::ParticleUpdateFunc RingUpdateFunc = [](ParticleManager::Particle& particle, float deltaTime) {
    particle.currentTime += deltaTime;
    float t = particle.currentTime / particle.lifeTime;

    // イージング（Cubic EaseOut: 最初は爆発的に速く、徐々に減速）
    float easeOut = 1.0f - std::powf(1.0f - t, 3);

    // 最大サイズ（例: 8.0f）まで一気に広げる
    float currentScale = easeOut * 8.0f;
    particle.transform.scale.x = currentScale;
    particle.transform.scale.z = currentScale; // 水平方向
    particle.transform.scale.y = 0.1f;         // 厚みは薄く保つ

    // 余韻を残しながら滑らかに消えていく（アルファフェードアウト）
    particle.color.w = 1.0f - easeOut; 
};