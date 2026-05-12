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
    box_ = Object3dCommon::GetInstance()->GetDefaultSkyBox();
    CreateCameraResource();
}
void Object3d::Update()
{


    if (Object3dCommon::GetInstance()->GetDefaultCamera() != camera_)
    {
        camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();


    }

    ////model_->Update();
    ////  WVP行列の作成
    //Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    //Matrix4x4 worldViewProjectionMatrix = {};
    ////ワールド行列とビュー行列とプロジェクション行列を掛け算
    //if (camera_)
    //{
    //    cameraData_->worldPosition = camera_->GetTranslate();
    //    worldViewProjectionMatrix = Multiply(Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix), camera_->GetViewProtectionMatrix());
    //    //   worldViewProjectionMatrix = Multiply( worldMatrix, camera_->GetViewProtectionMatrix());
    //} else {
    //    worldViewProjectionMatrix = Multiply(model_->GetModelData().rootNode.localMatrix, worldMatrix);
    //}
    ////行列をGPUに転送
    //wvpResource_->WVP = worldViewProjectionMatrix;
    //wvpResource_->World = worldMatrix;
    //wvpResource_->WorldInverseTranspose = Transpose(Inverse(worldMatrix));

    ImguiInstances();

    UpdateModelInstances();

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

  /*  if (camera_)
    {



    }*/


    Object3dCommon::GetInstance()->Object3dCommonDraw();
    auto psoSet = PSOManager::GetInstance()->GetPso(psoName_, blendMode_, fillMode_);

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());
    commandList->SetPipelineState(psoSet.pipelineState.Get());

    // PSOをセット
   // DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    //WVP行列リソースの設定
    //DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());
   // LightManager::GetInstance()->Draw(3);
    LightManager::GetInstance()->Draw(3);

    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(7, cameraResource_->GetGPUVirtualAddress());

    if (box_) {

        // SkyBoxクラスにテクスチャのGPUハンドルを取得するメソッドがあると仮定
        // 例: targetBox->GetSrvHandle()
        commandList->SetGraphicsRootDescriptorTable(8, TextureManager::GetInstance()->GetSrvHandleGPU(box_->GetTextureIndex()));
    }

    //light

    if (!models_.empty())
    {
        for (auto& instance : models_) {
            if (!instance->model) continue;

            // このインスタンス専用の WVP 行列バッファをセット
            commandList->SetGraphicsRootConstantBufferView(
                1, // kTransform
                instance->resource->GetGPUVirtualAddress()
            );

            if (instance->Active)
            {
                instance->model->Draw();

            }
            // インスタンスごとのモデル描画
        }
    }


    /*  if (model_) {
          model_->Draw();
      }*/
}

//void Object3d::SetModel(const std::string& filePath)
//{
//    model_ = ModelManager::GetInstance()->findModel(filePath);
//}
void Object3d::AddModel(const std::string& modelPath, const std::string& name, const std::string& parent)
{
    auto newInst = std::make_unique<ModelInstance>();
    newInst->model = ModelManager::GetInstance()->findModel(modelPath);
    newInst->name = name;
    if (!parent.empty())
    {
        newInst->parent = this->FindInstance(parent);

    }


    // インスタンス専用の定数バッファを作成 (DXCommonの機能を利用)
    newInst->resource = DXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));

    // 書き込み用アドレスを取得(Map)し、構造体のポインタに保存しておく
    newInst->resource->Map(0, nullptr, reinterpret_cast<void**>(&newInst->mappedData));

    // 行列の初期化
    newInst->mappedData->World = Makeidentity4x4();
    newInst->mappedData->WVP = Makeidentity4x4();

    // Object3dが管理するリストに追加
    models_.push_back(std::move(newInst));


}
Object3d::ModelInstance* Object3d::FindInstance(const std::string& name)
{
    for (auto& instance : models_) {
        if (instance->name == name) {
            return instance.get();
        }
    }
    return nullptr; // 見つからない場合はnullptrを返す
}

void Object3d::ImguiInstances()
{
#ifdef USE_IMGUI
    // ★パフォーマンス最適化: IMGUI の更新を条件付きで実行
    for (auto& instance : models_) {
        if (ImGui::TreeNode(instance->name.c_str())) {
            ImGui::DragFloat3("Scale", &instance->transform.scale.x, 0.01f);
            ImGui::DragFloat3("Rotate", &instance->transform.rotate.x, 0.5f);
            ImGui::DragFloat3("Translate", &instance->transform.translate.x, 0.1f);
            // ★マーク: ダーティフラグを設定して次フレームの更新を促す
            instance->isDirty = true;
            ImGui::TreePop();
        }
    }


#endif // USE_IMGUI
}

void Object3d::CreateWVPResource()
{
    //座標変換
    transformationMatrixResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidentity4x4();
    wvpResource_->World = Makeidentity4x4();
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


void Object3d::UpdateModelInstances()
{
    // 1. ベース行列（Object3d自身のトランスフォーム）の更新チェック
    bool baseMatrixChanged = memcmp(&transform_, &cachedBaseTransform_, sizeof(EulerTransform)) != 0;
    Matrix4x4 objectBaseMatrix;
    if (baseMatrixChanged || isBaseMatrixDirty_) {
        objectBaseMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
        cachedBaseTransform_ = transform_;
        isBaseMatrixDirty_ = false;
    } else {
        objectBaseMatrix = MakeAffineMatrix(cachedBaseTransform_.scale, cachedBaseTransform_.rotate, cachedBaseTransform_.translate);
    }

    // ★修正: カメラが存在する場合は毎フレーム更新（安全）
    bool cameraUpdated = (camera_ != nullptr);

    for (auto& instance : models_) {
        // ★修正: モデルのアニメーション更新を最初に実行


        if (instance->model) {

            instance->model->Update();
        }

        // ★追加: アニメーション適用後に transform を比較（重要）
        bool transformChanged = memcmp(&instance->transform, &instance->cachedTransform, sizeof(QuaternionTransform)) != 0;

        // ★修正: 以下の条件で行列を再計算
        // - トランスフォームが変更された
        // - ダーティフラグが立っている
        // - カメラが更新された（毎フレーム）
        // - または常に更新する（最も安全なアプローチ）
        if (transformChanged || instance->isDirty || cameraUpdated) {

            // ローカル行列を計算
            instance->localMatrix = MakeAffineMatrix(
                instance->transform.scale,
                instance->transform.rotate,
                instance->transform.translate
            );

            // 親子関係を考慮してワールド行列を計算
            if (instance->parent) {
                instance->worldMatrix = Multiply(instance->localMatrix, instance->parent->worldMatrix);
            } else {
                instance->worldMatrix = Multiply(instance->localMatrix, objectBaseMatrix);
            }

            // GPU に転送
            if (instance->mappedData && camera_) {
                instance->mappedData->World = instance->worldMatrix;
                instance->mappedData->WVP = Multiply(instance->worldMatrix, camera_->GetViewProtectionMatrix());
                instance->mappedData->WorldInverseTranspose = Transpose(Inverse(instance->worldMatrix));
            }

            // キャッシュを更新して次フレームの比較に備える
            instance->cachedTransform = instance->transform;
            instance->isDirty = false;
        }
    }
}
