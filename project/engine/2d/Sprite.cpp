#include "Sprite.h"
#include "SpriteCommon.h"
#include "MathFunction.h"
#include "TextureManager.h"
#include "DXCommon.h"
#include "Transform.h"

void Sprite::Initialize(std::string textureFilePath) {
    // UVトランスフォームの初期化（等倍、回転なし、ズレなし）
    uvTransform_.scale = { 1.0f, 1.0f };
    uvTransform_.rotate = 0.0f;
    uvTransform_.offset = { 0.0f, 0.0f };

    // 1. 頂点・インデックスバッファの作成とビューの設定 (4頂点 / 6インデックス)
    vertexRecourse_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * 4);
    indexResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);

    vertexBufferView_.BufferLocation = vertexRecourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation = indexResource_.Get()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT; // 32ビット整数インデックス

    // CPU側から書き込めるように常時マッピング
    vertexRecourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    indexResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    // 2. マテリアルバッファの作成と初期設定
    materialResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // デフォルトは白
    materialData_->enableLighting = false;                 // スプライトなのでライティングは原則OFF

    Matrix4x4 uvMatrix = MakeUVTransformMatrix(uvTransform_);
    materialData_->uvTransform = uvMatrix;

    // 3. 座標変換行列バッファの作成と初期設定
    transformationMatrixResourse_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    transformationMatrixData_->WVP = Makeidentity4x4();
    transformationMatrixData_->World = Makeidentity4x4();

    // 4. テクスチャの紐付けとサイズ自動調整
    //textureFilePath_ = textureFilePath;
    //textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

    //AdjustTextureSize();
    RegisterTexture(textureFilePath);
}

void Sprite::Update() {
    // アンカーポイントを考慮した基準となるローカル座標の計算
    float Left = 0.0f - anchorPoint_.x;
    float right = 1.0f - anchorPoint_.x;
    float top = 0.0f - anchorPoint_.y;
    float bottom = 1.0f - anchorPoint_.y;

    // 左右・上下反転の適用
    if (isFlipX_) {
        Left *= -1.0f;
        right *= -1.0f;
    }
    if (isFlipY_) {
        top *= -1.0f;
        bottom *= -1.0f;
    }

    // テクスチャのメタデータからUV座標（0.0〜1.0）を算出
    const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
    float tex_left = textureLeftTop.x / metadata.width;
    float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
    float tex_top = textureLeftTop.y / metadata.height;
    float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

    // 頂点データの更新 (左下・左上・右下・右上)
    // 左下
    vertexData_[0].position = { Left, bottom, 0.0f, 1.0f };
    vertexData_[0].texcord = { tex_left, tex_bottom };
    vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
    // 左上
    vertexData_[1].position = { Left, top, 0.0f, 1.0f };
    vertexData_[1].texcord = { tex_left, tex_top };
    vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
    // 右下
    vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
    vertexData_[2].texcord = { tex_right, tex_bottom };
    vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
    // 右上
    vertexData_[3].position = { right, top, 0.0f, 1.0f };
    vertexData_[3].texcord = { tex_right, tex_top };
    vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };

    // 三角形2枚分のインデックス設定（クワッド構成）
    indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
    indexData_[3] = 2; indexData_[4] = 1; indexData_[5] = 3;

    // 各種行列（ワールド、ビュー、平行投影プロジェクション）の合成
    EulerTransform transform{ {size_.x, size_.y, 1.0f}, {0.0f, 0.0f, rotation_}, {position_.x, position_.y, 0.0f} };

    Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    Matrix4x4 viewMatrix = Makeidentity4x4(); // スプライト用のカメラは恒等行列
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);

    // 行列を乗算して定数バッファへ転送 (World -> View -> Projection)
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;

    // UV変換行列を再計算してマテリアルバッファに反映
    Matrix4x4 uvMatrix = MakeUVTransformMatrix(uvTransform_);
    materialData_->uvTransform = uvMatrix;
}

void Sprite::Draw() {
    // 共通の描画前準備処理を呼び出し
    SpriteCommon::GetInstance()->SpriteCommonDraw();

    // 各種ブレンド・フィルモードに対応したPSO（パイプラインステート）の取得
    auto psoSet = PSOManager::GetInstance()->GetPso("Sprite", blendMode_, fillMode_);

    // パイプラインステートの設定
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());

    // 各種バッファビューの割り当て
    DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    DXCommon::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

    // 【ルートパラメータのバインド順の備忘録】
    // RootRegister[0]: マテリアル用 constantBuffer
    // RootRegister[1]: 座標変換行列用 constantBuffer
    // RootRegister[2]: テクスチャ用 DescriptorTable (SRV)

    // マテリアルバッファの設定 (ルートパラメータIndex: 0)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // 座標変換行列バッファの設定 (ルートパラメータIndex: 1)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourse_->GetGPUVirtualAddress());

    // テクスチャSRV（ディスクリプタテーブル）の設定 (ルートパラメータIndex: 2)
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));

    // インデックスを使用したプリミティブの描画（計6インデックス、1インスタンス）
    DXCommon::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetTextureByFilePath(const std::string& textureFilePath) {
    textureFilePath_ = textureFilePath; // パスを更新
    textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
}

size_t Sprite::RegisterTexture(const std::string& textureFilePath)
{
    // TextureManagerを介してGPU側のテクスチャインデックスを取得（未ロードならロードされる）
    uint32_t managerIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

    // 情報を格納
    registeredTextures_.push_back({ textureFilePath, managerIndex });

    // 最初に登録されたテクスチャをデフォルトとして適用
    if (registeredTextures_.size() == 1) {
        textureFilePath_ = textureFilePath;
        textureIndex_ = managerIndex;
        AdjustTextureSize();
    }

    // 登録された番号（配列のインデックス）を返す
    return registeredTextures_.size() - 1;
}

void Sprite::SetTextureByIndex(size_t index)
{
    // 範囲チェック
    if (index >= registeredTextures_.size()) {
        assert(false && "指定されたテクスチャインデックスは登録されていません。");
        return;
    }

    // カレントのテクスチャ情報を更新
    textureFilePath_ = registeredTextures_[index].filePath;
    textureIndex_ = registeredTextures_[index].managerIndex;

    // サイズを新しいテクスチャに合わせる
    AdjustTextureSize();
}

void Sprite::AdjustTextureSize() {
    // テクスチャのメタデータを取得して、スプライト全体のサイズを画像本来のサイズに合わせる
    const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

    textureSize.x = static_cast<float>(metadata.width);
    textureSize.y = static_cast<float>(metadata.height);
    size_ = textureSize;
}