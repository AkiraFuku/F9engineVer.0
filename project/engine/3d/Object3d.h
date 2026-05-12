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

    struct ModelInstance {
        std::string name; // 識別用
        std::shared_ptr<Model> model;
        //    EulerTransform transform = { {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} }; // そのパーツ独自のローカル座標
               // ★従来のEulerTransformに加えてQuaternionTransformを追加
        QuaternionTransform transform = {
            {1.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},  // 単位クォータニオン (x, y, z, w)
            {0.0f, 0.0f, 0.0f}
        };
        Matrix4x4 localMatrix;     // 計算後のローカル行列
        Matrix4x4 worldMatrix;     // 親を含めた最終的なワールド行列

        ModelInstance* parent = nullptr; // 親へのポインタ（親子関係用）
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        TransformationMatrix* mappedData = nullptr;

        bool Active = true; // インスタンスの有効/無効フラグ

        // ★パフォーマンス最適化: ダーティフラグの追加
        bool isDirty = true; // 初期状態は更新が必要//
        QuaternionTransform cachedTransform; // キャッシュされたトランスフォーム
        

    };

    void Initialize();
    void Update();
    void Draw();
   // void SetModel(const std::string& filePath);

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
    /*void SetAnimations(Animation* animation) {
        if (model_) {
            model_->SetAnimation(animation);
        }
    }
    void SetAnimationTime(float time) {
        if (model_) {
            model_->SetAnimationTime(time);
        }
    }*/
    void SetAnimations(const std::string& instanceName, Animation* animation) {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                instance->model->SetAnimation(animation);
                break;
            }
        }
    }   
    void SetAnimationTime(const std::string& instanceName, float time) {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                instance->model->SetAnimationTime(time);
                break;
            }
        }
    }
    
    void SetPsoName(const std::string& psoName) {
        psoName_ = psoName;
    }
    EulerTransform GetTransform() const {
        return transform_;
    }

     void SetModelInstanceColor(const std::string& instanceName, const Vector4& color) {
         for (const auto& instance : models_) {
             if (instance->name == instanceName) {
                 instance->model->SetColor(color);
                 break;
             }
         }
     }
     Vector4 GetModelInstanceColor(const std::string& instanceName) const {
         for (const auto& instance : models_) {
             if (instance->name == instanceName) {
                 return instance->model->GetColor();
             }
         }
         return {1.0f, 1.0f, 1.0f, 1.0f}; // デフォルトの白色
     }


    ///モデルインスタンスのゲッター
    const std::vector<std::unique_ptr<ModelInstance>>& GetModelInstances() const {
        return models_;
    }

    // モデルを追加する関数
    void AddModel(const std::string& modelPath, const std::string& name, const std::string& parent = {});

    void RemoveModel(const std::string& name);
    // 特定のモデルの座標を操作するゲッターなど
    ModelInstance* FindInstance(const std::string& name);

    void SetSkyBox(SkyBox* box) {
        box_ = box;
    }
    void SetInstanceUVTransform(const std::string& instanceName, const UVTransform& uvTransform) {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                instance->model->SetUVTransform(uvTransform);
                break;
            }
        }
    }
    UVTransform GetInstanceUVTransform(const std::string& instanceName) const {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                return instance->model->GetUVTransform();
            }
        }
        return {}; // デフォルトのUVTransform
    }
    Vector2 GetInstanceUVScale(const std::string& instanceName) const {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                return instance->model->GetUVScale();
            }
        }
        return {1.0f, 1.0f}; // デフォルトのスケール
    }
    float GetInstanceUVRotation(const std::string& instanceName) const {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                return instance->model->GetUVRotation();
            }
        }
        return 0.0f; // デフォルトの回転
    }
    Vector2 GetInstanceUVOffset(const std::string& instanceName) const {
        for (const auto& instance : models_) {
            if (instance->name == instanceName) {
                return instance->model->GetUVOffset();
            }
        }
        return {0.0f, 0.0f}; // デフォルトのオフセット
    }

private:
    void ImguiInstances();
    std::vector<std::unique_ptr<ModelInstance>> models_; // 複数のモデル実体
    //float radius_ = 1.0f;
   // std::shared_ptr<Model> model_ = nullptr;
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

    // ★パフォーマンス最適化: ベース行列のダーティフラグ
    bool isBaseMatrixDirty_ = true;
    EulerTransform cachedBaseTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    void UpdateModelInstances();


};

