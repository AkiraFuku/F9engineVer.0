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
#include "PSOMnager.h"
#include "MathFunction.h"
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

    void SetRadius(float radius) { radius_ = radius; }
private:

    float radius_ = 1.0f;
    std::shared_ptr<Model> model_ = nullptr;
    //WVP行列リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* wvpResource_ = nullptr;
    void CreateWVPResource();

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
    CameraForGPU* cameraData_ = nullptr;
    void  CreateCameraResource();
    //トランスフォーム
    Transform transform_ = {};
    //カメラ　
    Camera* camera_ = nullptr;

    FillMode fillMode_ = FillMode::kSolid;
    BlendMode blendMode_ = BlendMode::None;
};

