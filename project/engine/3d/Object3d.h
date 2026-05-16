#pragma once
#include <Vector4.h>
#include <Vector2.h>
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include "Model.h"
#include "Camera.h"
#include "Object3dCommon.h"
#include "PSOManager.h"
#include "MathFunction.h"
#include "SkyBox.h"
class Animation;
class Object3d
{

public:

    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
        Matrix4x4 WorldInverseTranspose;
    };
    struct DirectionalLight {
        Vector4 color;//ライトの色
        Vector3 direction;//ライトの向き
        float intensity;// 明るさ


    };
    struct CameraForGPU
    {
        Vector3 worldPosition;
        float farClip;
        Vector3 cameraForward; // ★追加: カメラの前方ベクトル
        float padding;
    };
    void Initialize();
    void Update();
    void Draw();
    void SetModel(const std::string& filePath);

    //トランスフォームセッター
    void SetScale(const Vector3& scale) {
        transform_.scale = scale;
    }
    void SetRotate(const Vector3& rotate) {
        transform_.rotate = rotate;
    }
    void SetTranslate(const Vector3& translate) {
        transform_.translate = translate;
    }
    void SetCamera(Camera* camera) {
        camera_ = camera;
    }


    //トランスフォームゲッター
    const Vector3& GetScale()const {
        return transform_.scale;
    }
    const Vector3& GetRotate()const {
        return transform_.rotate;
    }
    const Vector3& GetTranslate()const {
        return transform_.translate;
    }

    void SetBlendMode(BlendMode blendMode) {
        blendMode_ = blendMode;
    }
    void SetFillMode(FillMode fillMode) {
        fillMode_ = fillMode;
    }
    void SetAnimations(Animation* animation) {
        if (model_) {
            model_->SetAnimation(animation);
        }
    }
    void SetAnimationTime(float time) {
        if (model_) {
            model_->SetAnimationTime(time);
        }
    }
    void SetPsoName(const std::string& psoName) {
        psoName_ = psoName;
    }
    EulerTransform GetTransform() const {
        return transform_;
    }

    void SetModelColor(const Vector4& color) {
        if (model_) {
            model_->SetColor(color);
        }
    }
    Vector4 GetModelInstanceColor() const {
        if (model_) {
            return    model_->GetColor();
        }
        return { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトの白色
    }






    void SetSkyBox(SkyBox* box) {
        box_ = box;
    }
    void SetUVTransform(const UVTransform& uvTransform) {
        if (model_)
        {
            model_->SetUVTransform(uvTransform);
        }
    }
    UVTransform GetUVTransform() const {
        if (model_)
        {
            return  model_->GetUVTransform();
        }
        return {}; // デフォルトのUVTransform
    }
    Vector2 GetUVScale() const {
        if (model_)
        {
            return  model_->GetUVScale();
        }
        return { 1.0f, 1.0f }; // デフォルトのスケール
    }
    float GetUVRotation() const {
         if (model_)
        {
            return  model_->GetUVRotation();
        }
        return 0.0f; // デフォルトの回転
    }
    Vector2 GetUVOffset() const {
       if (model_)
        {
            return  model_->GetUVOffset();
        }
        return { 0.0f, 0.0f }; // デフォルトのオフセット
    }

private:

    //float radius_ = 1.0f;
    std::shared_ptr<Model> model_ = nullptr;
    //WVP行列リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* wvpResource_ = nullptr;
    void CreateWVPResource();

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
    CameraForGPU* cameraData_ = nullptr;
    void  CreateCameraResource();

    //トランスフォーム
    EulerTransform transform_ = {};
    //カメラ　
    Camera* camera_ = nullptr;
    //スカイボックス
    SkyBox* box_ = nullptr;

    FillMode fillMode_ = FillMode::kSolid;
    BlendMode blendMode_ = BlendMode::None;

    std::string psoName_ = "Object3d";

};

