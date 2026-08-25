#pragma once
#include "Vector4.h"
#include "ParticleManager.h"
#include "Transform.h"

// ParticleGroupSet に対応した ParticleEmitter
// コンストラクタに setName を追加し、Emit/Update も setName + groupName でアクセスする
class ParticleEmitter
{
public:
    // setName  : ParticleGroupSet の名前
    // groupName: そのセット内の ParticleGroup の名前
    ///summary>
    /// ParticleEmitter クラスのコンストラクタ
    /// </summary>
    /// <param name="setName">ParticleGroupSet の名前</param>
    /// <param name="groupName">そのセット内の ParticleGroup の名前</param>
    /// <param name="transform">エミッタのトランスフォーム</param>
    /// <param name="count">1 回の発生数</param>
    /// <param name="frequency">発生間隔（秒）</param>
    /// <param name="frequencyTime">経過タイマー</param>
    /// <returns>なし</returns>

    ParticleEmitter(
        const std::string& setName,
        const std::string& groupName,
        EulerTransform transform,
        uint32_t count,
        float frequency,
        float frequencyTime
    );

    void SetTransform(const EulerTransform& transform) {
        worldTransform_ = transform;
    }
    void SetTranslate(const Vector3& translate) {
        worldTransform_.translate = translate;
    }

    // 毎フレーム呼ぶ（frequencyTime_ が frequency_ を超えたら自動 Emit）
    void Update();

    // 即座に 1 回 Emit する
    void Emit();

private:
    EulerTransform worldTransform_;  // エミッタのトランスフォーム
    uint32_t count_;            // 1 回の発生数
    float frequency_;           // 発生間隔（秒）
    float frequencyTime_;       // 経過タイマー
    std::string setName_;       // ParticleGroupSet 名
    std::string groupName_;     // ParticleGroup 名
};
