#pragma once
#include <vector>
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#include "Vector4.h"
#include "Vector3.h"
#include <memory>
#include <queue>

#include "PSOManager.h"

class Camera;

class PrimitiveDrawer {
public:

    struct VertexData {
        Vector4 position;
        Vector4 color;
    };

    struct TransformationMatrix {
        Matrix4x4 WVP;
    };

    static PrimitiveDrawer* GetInstance();
    friend struct std::default_delete<PrimitiveDrawer>;
    
    virtual ~PrimitiveDrawer() = default;
    
    virtual void Initialize();
    virtual void Finalize();
    
    void SetCamera(Camera* camera) {
        camera_ = camera;
    }
    
    virtual void Draw();

protected:
    PrimitiveDrawer() = default;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    VertexData* vertexData_ = nullptr;
    
    size_t vertexCount_ = 0;
    size_t maxVertexCount_ = 0;

    Camera* camera_ = nullptr;

    virtual void AddPSO();
    
    // サブクラス用の保護されたメソッド
    void UpdateVertexBuffer(const std::vector<VertexData>& vertices);
    void ClearVertices();

private:
    static std::unique_ptr<PrimitiveDrawer> instance_;
};