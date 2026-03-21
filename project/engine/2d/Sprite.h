#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include <wrl.h>
#include <d3d12.h>
#include<string>

#include "PSOManager.h"
class Sprite
{
public:
    struct VertexData {
        Vector4 position; // 4D position vector
        Vector2 texcord; // 2D texture coordinate vector
        Vector3 normal;
    };
    struct Material
    {
        Vector4 color;
        int32_t enableLighting;
        float padding[3]; // パディングを追加してサイズを揃える
        Matrix4x4 uvTransform; // UV変換行列

    };
    struct TransformationMatrix
    {
        Matrix4x4 WVP;
        Matrix4x4 World;

    };

    void Initialize( std::string textureFilePath);
    void Update();
    void Draw();

    const Vector2& GetPosition() const {
        return position_;
    }
    void SetPosition(const Vector2& position) {
        position_ = position;
    }

    float GetRotation() const {
        return rotation_;
    }
    void SetRotation(const float rotation) {
        rotation_ = rotation;
    }

    Vector4& GetColor() const {
        return materialData_->color;
    }
    void SetColor(const Vector4& color) {
        materialData_->color = color;
    }

    Matrix4x4& GetUV()const {
        return materialData_->uvTransform;
    }
    void SetUV(Matrix4x4& uvTransform) {
        materialData_->uvTransform = uvTransform;
    }

    const Vector2& GetSize()const {
        return size_;
    }
    void SetSize(const Vector2& Size) {
        this->size_ = Size;
    }

    const Vector2& GetAnchorPoint()const {
        return anchorPoint_;
    }
    void SetAnchorPoint(const Vector2& anchorPoint) {
        anchorPoint_ = anchorPoint;
    }

    bool GetIsFlipX()const {
        return isFlipX_;
    }
    void SetIsFlipX(bool isFlipX) {
        isFlipX_ = isFlipX;
    }
    bool GetIsFlipY()const {
        return isFlipY_;
    }
    void SetIsFlipY(bool isFlipY) {
        isFlipY_ = isFlipY;
    }

    Vector2 GetTextureLeftTop()const {
        return textureLeftTop;
    }
    void SetTextureLeftTop(const Vector2& textureLeftTop) {
        this->textureLeftTop = textureLeftTop;
    }
    Vector2 GetTextureSize()const {
        return textureSize;
    }
    void SetTextureSize(const Vector2& textureSize) {
        this->textureSize = textureSize;
    }
    void SetBlendMode(BlendMode blendMode) {
        blendMode_ = blendMode;
    }
    BlendMode GetBlendMode() const {
        return blendMode_;
    }
    void SetFillMode(FillMode fillMode) { fillMode_ = fillMode; }
    //テクスチャ変更
    void SetTextureByFilePath(const std::string& textureFilePath);

private:
    void AdjustTextureSize();
    BlendMode blendMode_ = BlendMode::Normal;
private:
    

    Vector2 position_ = { 0.0f,0.0f };
    float rotation_ = 0.0f;

    Vector2 size_ = { 10.0f,10.0f };

    Vector2 anchorPoint_ = { 0.0f,0.0f };

    bool isFlipX_ = false;
    bool isFlipY_ = false;

    //テクスチャ左上
    Vector2 textureLeftTop = { 0.0f,0.0f };
    //テクスチャ切り出しサイズ
    Vector2 textureSize{ 100.0f,100.0f };


    //buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourse_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    VertexData* vertexData_ = nullptr;
    uint32_t* indexData_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_;
    //マテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
    //座標変換
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourse_;
    TransformationMatrix* transformationMatrixData_ = nullptr;
    uint32_t textureIndex_ = 0;
    std::string textureFilePath_;
    FillMode fillMode_ = FillMode::kSolid;
};

