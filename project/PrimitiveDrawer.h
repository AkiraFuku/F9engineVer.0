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

    void DrawSphere(const Sphere& sphere, const Vector4& color, FillMode fillMode = FillMode::kSolid, BlendMode blendMode = BlendMode::Normal);
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
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    VertexData* sharedMappedPtr_ = nullptr; // マップ済みポインタ
    int32_t currentVertexOffset_ = 0;
    //std::vector<VertexData> vertices;
    static const uint32_t kMaxVertices = 128 * 1024; // 最大頂点数（必要に応じて調整）
    Microsoft::WRL::ComPtr<ID3D12Resource> WVPResource_;
    WVPMatrix* wvpData_ = nullptr;
    void WVPResourceCreate();

    Camera* camera_ = nullptr;

    // トポロジの種類
    enum class PrimithiveType {
        kLine,
        kTriangle,
        kPoint,
        kGrid,
        kPlane,
        kSphere,
        kAABB,
        kCount // 種類の総数
    };

    // トポロジごとに必要なリソース一式
    struct PrimitiveBatch {


        D3D_PRIMITIVE_TOPOLOGY d3dTopology;
        Toporogy psoTopology; // PSOManager用
    };

    // トポロジごとのバッチ管理
    std::map<PrimithiveType, PrimitiveBatch> batches_;
    struct DrawCommand {
        PrimithiveType type;
        uint32_t vertexCount;
        uint32_t startIndex; // 全体バッファ内の開始位置
        FillMode fillMode=FillMode::kSolid;
        BlendMode blendMode=BlendMode::Normal;
    };

    std::vector<DrawCommand> drawCommands_; // 描画待ちのリスト

};