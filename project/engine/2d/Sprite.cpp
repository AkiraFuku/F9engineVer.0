#include "Sprite.h"
#include "SpriteCommon.h"
#include "MathFunction.h"
#include "TextureManager.h"
#include "DXCommon.h"
#include "Transform.h"
void Sprite::Initialize(std::string textureFilePath) {



    vertexResourse_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(VertexData) * 4);
    indexResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(uint32_t) * 6);
    vertexBufferView_.BufferLocation =
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation =
        indexResource_.Get()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;//32ビット整数

    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    indexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    //
    materialResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    //データの設定
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidetity4x4();
    //座標変換
    transformationMatrixResourse_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    transformationMatrixData_->WVP = Makeidetity4x4();
    transformationMatrixData_->World = Makeidetity4x4();

    textureFilePath_ = textureFilePath;
    //テクスチャの読み込み
    textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

    AdjustTextureSize();
}
void Sprite::Update() {

    float Left = 0.0f - anchorPoint_.x;
    float right = 1.0f - anchorPoint_.x;
    float top = 0.0f - anchorPoint_.y;
    float bottom = 1.0f - anchorPoint_.y;

    if (isFlipX_)
    {
        Left *= -1.0f;
        right *= -1.0f;
    }
    if (isFlipY_)
    {
        top *= -1.0f;
        bottom *= -1.0f;
    }

    const DirectX::TexMetadata& metadata =
        TextureManager::GetInstance()->GetMetaData(textureFilePath_);

    float tex_left = textureLeftTop.x / metadata.width;
    float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
    float tex_top = textureLeftTop.y / metadata.height;
    float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

    // 左下
    vertexData_[0].position = { Left, bottom, 0.0f, 1.0f };
    vertexData_[0].texcord = { tex_left, tex_bottom };
    vertexData_[0].normal = { 0.0f,0.0f, -1.0f };
    // 左上
    vertexData_[1].position = { Left, top, 0.0f, 1.0f };
    vertexData_[1].texcord = { tex_left, tex_top };
    vertexData_[1].normal = { 0.0f,0.0f, -1.0f };
    // 右下
    vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
    vertexData_[2].texcord = { tex_right, tex_bottom };
    vertexData_[2].normal = { 0.0f,0.0f, -1.0f };
    // 右上
    vertexData_[3].position = { right, top, 0.0f, 1.0f };
    vertexData_[3].texcord = { tex_right, tex_top };
    vertexData_[3].normal = { 0.0f,0.0f, -1.0f };

    indexData_[0] = 0;
    indexData_[1] = 1;
    indexData_[2] = 2;
    indexData_[3] = 2;
    indexData_[4] = 1;
    indexData_[5] = 3;



    EulerTransform transform{ {size_.x,size_.y,1.0f},{0.0f,0.0f,rotation_},{position_.x,position_.y,0.0f} };

    Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 viewMatrix = Makeidetity4x4();
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
    //スプライトのワールド行列とビュー行列とプロジェクション行列を掛け算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;


}

void Sprite::Draw()
{
    SpriteCommon::GetInstance()->SpriteCommonDraw();
    //Object3dCommon::GetInstance()->Object3dCommonDraw();
    auto psoSet = PSOManager::GetInstance()->GetPso("Sprite", blendMode_, fillMode_);

    // PSOをセット
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    //パイプラインステートとルートシグネチャの設定
    DXCommon::GetInstance()->
        GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    //インデックスバッファの設定
    DXCommon::GetInstance()->
        GetCommandList()->IASetIndexBuffer(&indexBufferView_);
    //マテリアルの設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // SRVのディスクプリプターテーブルの先頭を設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));

    //座標変換行列の設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(1, transformationMatrixResourse_->GetGPUVirtualAddress());

    DXCommon::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetTextureByFilePath(const std::string& textureFilePath)
{
    textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

void Sprite::AdjustTextureSize()
{
    //メタデータの取得
    const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

    textureSize.x = static_cast<float>(metadata.width);
    textureSize.y = static_cast<float>(metadata.height);
    size_ = textureSize;
}
