#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <d3dx12.h> // CD3DX12 構造体のために推奨
#include <dxcapi.h> // IDxcBlobのために推奨


// 追加：シェーダーの種類を定義
enum class ShaderType { VS, PS, GS, HS, DS, CS };

struct ShaderSet {
    // vectorで管理する場合。インデックス = ShaderType とすると扱いやすいです。
    // もしくは、個別に保持せず map<ShaderType, ComPtr<IDxcBlob>> にするのも手です。
    std::unordered_map<ShaderType, Microsoft::WRL::ComPtr<IDxcBlob>> blobs;
};

struct InputLayout
{
    D3D12_INPUT_LAYOUT_DESC inputLayout{};
    std::vector<D3D12_INPUT_ELEMENT_DESC>inputElement{};
};
struct PsoConfig {
  struct ShaderPath {
        ShaderType type;
        std::wstring path;
        std::string entryPoint = "main"; // 必要に応じて
        std::wstring profile;           // L"vs_6_0" など
    };
    std::vector<ShaderPath> shaderPaths;
    
    using RootSignatureGenerator = std::function<Microsoft::WRL::ComPtr<ID3D12RootSignature>()>;
    RootSignatureGenerator rootSignatureGenerator;   

    using InputLayoutGenerator = std::function<InputLayout()>;
    InputLayoutGenerator inputLayoutGenerator;

    D3D12_DEPTH_STENCIL_DESC depth = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE;
    bool depthEnable = true;
    D3D12_DEPTH_WRITE_MASK depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    D3D12_COMPARISON_FUNC depthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

};
enum class Toporogy{ PointList,LineList,TriangleList,};
enum class BlendMode { None, Normal, Add, Subtract, Multiply, Screen };
enum class FillMode { kSolid, kWireFrame };



struct PsoSet {
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};

class PSOManager {
public:
    static PSOManager* GetInstance();
      friend struct std::default_delete<PSOManager>;
    void Initialize();
    void Finalize();

    void RegisterPsoGenerator(const std::string& name, const PsoConfig& psoConfig);
    const PsoSet& GetPso(const std::string& name, BlendMode blendMode = BlendMode::None, FillMode fillMode = FillMode::kSolid,Toporogy type=Toporogy::TriangleList);

    D3D12_STATIC_SAMPLER_DESC StaticSamplers();

private:
    PSOManager() = default;
    ~PSOManager() = default;

    void CreatePso(const std::string& name, BlendMode blend, FillMode fill,Toporogy type);
    D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode);
    void EnsureShaders(const std::string& name,ShaderSet& outSet);
    D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(Toporogy type);

    struct CacheKey {
        std::string name;
        BlendMode blend;
        FillMode fill;
        Toporogy type;
        bool operator==(const CacheKey& o) const {
            return name == o.name && blend == o.blend && fill == o.fill&& type==o.type;
        }
    };

    struct KeyHasher {
        std::size_t operator()(const CacheKey& k) const {
            // 全ての要素をハッシュ計算に含める
            size_t h1 = std::hash<std::string>()(k.name);
            size_t h2 = std::hash<int>()((int)k.blend);
            size_t h3 = std::hash<int>()((int)k.fill);
            size_t h4 = std::hash<int>()((int)k.type);
            return h1 ^ (h2 << 1) ^ (h3 << 2)^(h4<<3);
        }
    };

    static std::unique_ptr<PSOManager> instance_;
    std::unordered_map<std::string, PsoConfig> psoConfigs_;
    std::unordered_map<CacheKey, PsoSet, KeyHasher> psoCache_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSigCache_;
    std::unordered_map<std::string, ShaderSet> shaderCache_;
};