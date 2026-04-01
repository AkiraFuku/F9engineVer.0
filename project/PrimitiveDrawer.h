#pragma once
#include <vector>
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#include "Vector4.h"
#include "Vector3.h"
#include <memory>

#include "PSOManager.h"

class Camera;

class PrimitiveDrawer {
public:
    struct VertexData {
        Vector4 position;
    };

    struct Material {
        Vector4 color;
    };

    struct TransformationMatrix {
        Matrix4x4 WVP;
    };

    static PrimitiveDrawer* GetInstance();
    friend struct std::default_delete<PrimitiveDrawer>;
    void Initialize();
    void Finalize();
    void SetCamera(Camera* camera) {
        camera_ = camera;
    }
    void Draw();


    //void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

    //void DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& color);

    //void ExecuteDraw();

private:
    PrimitiveDrawer() = default;
    ~PrimitiveDrawer() = default;
    static std::unique_ptr<PrimitiveDrawer> instance_;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    VertexData* vertexData_ = nullptr;

    void AddPSO();


    //Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    //Material* materialData_ = nullptr;



    Camera* camera_ = nullptr;

};