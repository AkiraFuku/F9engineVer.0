#pragma once
#include <vector>
#include <wrl/client.h>
#include <d3d12.h>
#include "Vector4.h"
#include "Vector3.h"
#include <memory>

#include "PSOManager.h"

#include <map>

#include"DrawFunction.h"
class Camera;

class PrimitiveDrawer {
public:

    struct VertexData {
        Vector4 position;
        Vector4 color;
    };

    /*struct Material {

    };*/

    struct WVPMatrix {
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


    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
    void DrawTriangle(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector4& color, FillMode fillMode = FillMode::kSolid, BlendMode blendMode = BlendMode::Normal);

    void DrawSphere(const Sphere& sphere, const Vector4& color);
    void DrawGrid(); // 引数は内部のカメラ行列を使うため不要に
    void DrawAABB(const AABB& aabb, const Vector4& color);
    void DrawSegment(const Segment& segment, const Vector4& color);
    void DrawPlane(const Plane& plane, const Vector4& color);

private:
    PrimitiveDrawer() = default;
    ~PrimitiveDrawer() = default;
    static std::unique_ptr<PrimitiveDrawer> instance_;

    /*  Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
      D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
      VertexData* vertexData_ = nullptr;*/

    void AddPSO();


private:
    //  std::vector<VertexData> vertices_; // 描画予約された頂点リスト
    static const uint32_t kMaxVertices = 4096; // 最大頂点数（必要に応じて調整）
    Microsoft::WRL::ComPtr<ID3D12Resource> WVPResource_;
    WVPMatrix* wvpData_ = nullptr;
    void WVPResourceCreate();

    Camera* camera_ = nullptr;

    // トポロジの種類
    enum class TopologyType {
        kLine,
        kTriangle,
        kPoint,
        kCount // 種類の総数
    };
    enum class Primithive
    {
        kLine,
        kTriangle,
        kGrid,
        kPlane,
        kSphere,
        kAABB
        

    };

    // トポロジごとに必要なリソース一式
    struct PrimitiveBatch {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_VERTEX_BUFFER_VIEW vbv;
        std::vector<VertexData> vertices;
        D3D_PRIMITIVE_TOPOLOGY d3dTopology;
        Toporogy psoTopology; // PSOManager用
        FillMode fillMode = FillMode::kSolid; // 必要に応じて追加
        BlendMode blendMode = BlendMode::Normal; // 必要に応じて追加
    };
    
    // トポロジごとのバッチ管理
    std::map<Primithive, PrimitiveBatch> batches_;

};