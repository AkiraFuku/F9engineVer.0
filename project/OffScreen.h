#pragma once
#include <d3d12.h>     // 先にDirectXの定義を入れる
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "Vector4.h"

class Camera;
class OffScreen
{
public:
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

private:
    OffScreen() = default;
    ~OffScreen() = default;

    static std::unique_ptr<OffScreen> instance;

    Camera* camera_;

    struct Material
    {
        Matrix4x4 projectionInverse;
    };
    Material* materialData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialConstantBuffer_;
    
    // 前回のコードでこれらがエラーになっていた場合、ここに含まれる型が
    // <d3d12.h> を読み込むことで解決されます
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};