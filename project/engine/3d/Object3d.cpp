#include "Object3d.h"
#include "Object3dCommon.h"
#include <cassert>
#include <fstream> // 追加: ifstreamの完全な型を利用するため
#include <sstream> // 追加: istringstreamのため
#include "MathFunction.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include <imgui.h>
#include "LightManager.h"

void Object3d::Initialize()
{

    //WVP行列リソースの作成
    CreateWVPResource();
    //平行光源リソースの作成
   // CreateDirectionalLightResource();
    transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
    camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
    CreateCameraResource();
}
void Object3d::Update()
{
    model_->Update();
    //  WVP行列の作成
    Matrix4x4 worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 worldViewProjectionMatrix = {};
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    if (camera_)
    {
        cameraData_->worldPosition = camera_->GetTranslate();
      worldViewProjectionMatrix = Multiply( Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix), camera_->GetViewProtectionMatrix());
     //   worldViewProjectionMatrix = Multiply( worldMatrix, camera_->GetViewProtectionMatrix());
    } else {
        worldViewProjectionMatrix =Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix);
    }
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
    wvpResource_->WorldInverseTranspose= Inverse(worldMatrix);
}

void Object3d::Draw()
{
    // カメラがセットされていない場合は描画できないので終了
    if (!camera_) return;

    // ==========================================================
    // 視錐台カリング (Frustum Culling) の処理
    // ==========================================================
    
    // 1. オブジェクトのワールド座標を取得
    // (ここでは簡易的にTransformのtranslateを使用します)
    Vector3 worldPos = transform_.translate;

    // 2. カメラの ViewProjection行列 を取得
    // (Cameraクラスのメソッド名 "ViewProtection" に合わせています)
    Matrix4x4 vpMatrix = camera_->GetViewProtectionMatrix();

    // 3. 座標変換 (ワールド空間 -> クリップ空間)
    // 以下の計算は MathFunction に Vector3 * Matrix4x4 の関数があればそれを使ってください。
    // ここでは仕組みを明確にするため展開して記述します。
    // (DirectX/Row-Major 前提: v * M)
    Vector4 clipPos;
    clipPos.x = worldPos.x * vpMatrix.m[0][0] + worldPos.y * vpMatrix.m[1][0] + worldPos.z * vpMatrix.m[2][0] + 1.0f * vpMatrix.m[3][0];
    clipPos.y = worldPos.x * vpMatrix.m[0][1] + worldPos.y * vpMatrix.m[1][1] + worldPos.z * vpMatrix.m[2][1] + 1.0f * vpMatrix.m[3][1];
    clipPos.z = worldPos.x * vpMatrix.m[0][2] + worldPos.y * vpMatrix.m[1][2] + worldPos.z * vpMatrix.m[2][2] + 1.0f * vpMatrix.m[3][2];
    clipPos.w = worldPos.x * vpMatrix.m[0][3] + worldPos.y * vpMatrix.m[1][3] + worldPos.z * vpMatrix.m[2][3] + 1.0f * vpMatrix.m[3][3];

    // 4. 判定 (簡易的な Sphere vs Frustum)
    // クリップ空間での視界範囲: -w <= x <= w, -w <= y <= w, 0 <= z <= w
    // オブジェクトの半径(radius_)も考慮してマージンを取ります
    bool isVisible = true;

    // 右・左
    if (clipPos.x < -clipPos.w - radius_ || clipPos.x > clipPos.w + radius_) {
        isVisible = false;
    }
    // 上・下
    else if (clipPos.y < -clipPos.w - radius_ || clipPos.y > clipPos.w + radius_) {
        isVisible = false;
    }
    // 前・後 (DirectXは 0 <= z <= w です)
    // 手前(Near)より前、または奥(Far)より後ろにあるか
    // ※ z < 0 - radius_ だとカメラの真後ろすぐが消えるため、少し余裕を持たせるか、
    // 厳密にやるならNear平面との距離判定が必要です。ここでは簡易判定です。
    else if (clipPos.z < 0.0f - radius_ || clipPos.z > clipPos.w + radius_) {
        isVisible = false;
    }

    // 画面外なら描画処理をスキップ
    if (!isVisible) {
        return;
    }

    Object3dCommon::GetInstance()->Object3dCommonDraw();
    PsoProperty psoProp{};
    psoProp.type = PipelineType::Object3d;
    psoProp.blendMode = blendMode_;
    psoProp.fillMode = fillMode_;
    psoProp.depthMode = depthMode_;
    PsoSet psoSet = PSOMnager::GetInstance()->GetPsoSet(psoProp);

    // PSOをセット
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    //WVP行列リソースの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());
    //light
    LightManager::GetInstance()->Draw(3);
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(7, cameraResource_->GetGPUVirtualAddress());
    if (model_) {
        model_->Draw();
    }
}

void Object3d::SetModel(const std::string& filePath)
{
    model_ = ModelManager::GetInstance()->findModel(filePath);
}


void Object3d::CreateWVPResource()
{
    //座標変換
    transformationMatrixResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidetity4x4();
    wvpResource_->World = Makeidetity4x4();
    wvpResource_->WorldInverseTranspose = Inverse(wvpResource_->World);
}



void Object3d::CreateCameraResource()
{
    cameraResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource((sizeof(CameraForGPU) + 0xff) & ~0xff);
    cameraResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
    if (camera_ != nullptr)
    {
        cameraData_->worldPosition = camera_->GetTranslate();
    } else
    {
        cameraData_->worldPosition = Vector3{ 1.0f,1.0f,1.0f };
    }

}
