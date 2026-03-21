#pragma once
#include "Vector3.h"
#include "Quanternion.h"
#include <map>
#include <vector>
#include <string>


class Animation
{
public:

    template <typename tValue>
    struct KeyFrame
    {
        tValue value;
        float time;
    };
    using KeyFrameVector3 = KeyFrame<Vector3>;
    using KeyFrameQuaternion = KeyFrame<Quaternion>;


    template <typename tValue>
    struct AnimationCurve
    {
        std::vector<KeyFrame<tValue>> keyFrames;
    };

    struct  NodeAnimation {
        AnimationCurve<Vector3> translate;
        AnimationCurve<Quaternion> rotate;
        AnimationCurve<Vector3> scale;
    };

    struct AnimationData
    {
        float duration; // アニメーションの全体の長さ（秒）
        std::map<std::string, NodeAnimation> nodeAnimations; // ノード名とそのアニメーションデータのマップ
    };

 static  AnimationData LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
 void Initialize(const std::string& directryPath, const std::string& filename);
    void Update();


     void Draw();
     AnimationData GetAnimationData() const {
         return AnimeData_;
     }


     void SetCurrentTime(float time) {
         currentTime_ = time;
     }
     float GetCurrentTime_() const {
         return currentTime_;
     }


     Vector3 CalculateValue(const std::vector<KeyFrameVector3>& keyframes, float time);

    Quaternion CalculateValue(const std::vector<KeyFrameQuaternion>& keyframes, float time);

private:
    AnimationData AnimeData_;

    float currentTime_ = 0.0f; // 現在のアニメーション時間



};



