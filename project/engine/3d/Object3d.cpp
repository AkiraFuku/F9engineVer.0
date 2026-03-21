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
        worldViewProjectionMatrix = Multiply(Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix), camera_->GetViewProtectionMatrix());
        //   worldViewProjectionMatrix = Multiply( worldMatrix, camera_->GetViewProtectionMatrix());
    } else {
        worldViewProjectionMatrix = Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix);
    }
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
    wvpResource_->WorldInverseTranspose = Transpose(Inverse(worldMatrix));

    if (camera_ && cameraData_)
    {
        cameraData_->worldPosition = camera_->GetTranslate();
        cameraData_->farClip = camera_->GetFarCrip(); // ★ここを追加
        // ★追加: カメラのワールド行列から前方ベクトル(Z軸)を抽出
        // 一般的な行優先(Row-Major)の4x4行列の場合、3行目(m[2])がZ軸(Forward)です
        const Matrix4x4& mat = camera_->GetWorldMatrix();
        // 正規化されているはずですが、念のため正規化して送ると安全です
        Vector3 forward = { mat.m[2][0], mat.m[2][1], mat.m[2][2] };
        cameraData_->cameraForward = Normalize(forward);
    }
}

void Object3d::Draw()
{
    // カメラがセットされていない場合は描画できないので終了
    if (!camera_) return;


    Object3dCommon::GetInstance()->Object3dCommonDraw();
    auto psoSet = PSOManager::GetInstance()->GetPso(psoName_, blendMode_, fillMode_);

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());
    commandList->SetPipelineState(psoSet.pipelineState.Get());

    // PSOをセット
   // DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
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
        cameraData_->farClip = camera_->GetFarCrip(); // ★ここを追加

        // ★追加: カメラのワールド行列から前方ベクトル(Z軸)を抽出
        // 一般的な行優先(Row-Major)の4x4行列の場合、3行目(m[2])がZ軸(Forward)です
        const Matrix4x4& mat = camera_->GetWorldMatrix();
        // 正規化されているはずですが、念のため正規化して送ると安全です
        Vector3 forward = { mat.m[2][0], mat.m[2][1], mat.m[2][2] };
        cameraData_->cameraForward = Normalize(forward);

    } else
    {
        cameraData_->worldPosition = Vector3{ 1.0f,1.0f,1.0f };
        cameraData_->farClip = 1000.0f;
        cameraData_->cameraForward = Vector3{ 0.0f,0.0f,1.0f };

    }

}
