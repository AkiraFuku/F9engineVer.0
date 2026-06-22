#pragma once
#include <d3d12.h>     // 先にDirectXの定義を入れる
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "Vector4.h"
#include "Vector2.h"


struct BlurParam
{
    Vector2 center;
    int32_t radius;
    float blurWidth;

};

class Camera;
class OffScreen
{
public:

    struct MaskMaterial
    {
        std::string textureFilePath;
        uint32_t maskTextureSrvIndex;
    };

    static OffScreen* GetInstance();

    OffScreen(const OffScreen&) = delete;
    OffScreen& operator=(const OffScreen&) = delete;
    friend struct std::default_delete<OffScreen>;

    void Initialize();
    void Finalize();
    void Draw();

    void SetCamera(Camera* camera) {
        camera_ = camera;
    }

    void SetMaskMaterial( std::string textureFilePath) ;

private:
    OffScreen() = default;
    ~OffScreen() = default;

    static std::unique_ptr<OffScreen> instance;

    Camera* camera_;

    BlurParam* blurParamData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> blurConstantBuffer_;

    struct Material
    {
        Matrix4x4 projectionInverse;
    };
    Material* materialData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialConstantBuffer_;

    MaskMaterial MaskMaterial_;

    struct DissolveParm
    {
        float threshold;
        float padding[3]; // 16バイトアラインメントのためのパディング
    };
    DissolveParm* dissolveParamData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> dissolveConstantBuffer_;

    // 前回のコードでこれらがエラーになっていた場合、ここに含まれる型が
    // <d3d12.h> を読み込むことで解決されます
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};