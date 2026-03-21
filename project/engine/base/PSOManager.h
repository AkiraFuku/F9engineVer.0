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


struct ShaderSet {
    Microsoft::WRL::ComPtr<IDxcBlob> vs;
    Microsoft::WRL::ComPtr<IDxcBlob> ps;
};

struct PsoConfig {
    std::wstring vsPath;
    std::wstring psPath;
    
    using RootSignatureGenerator = std::function<Microsoft::WRL::ComPtr<ID3D12RootSignature>()>;
    RootSignatureGenerator rootSignatureGenerator;   

    using InputLayoutGenerator = std::function<std::vector<D3D12_INPUT_ELEMENT_DESC>()>;
    InputLayoutGenerator inputLayoutGenerator;

    D3D12_DEPTH_STENCIL_DESC depth = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
    bool depthEnable = true;
    D3D12_DEPTH_WRITE_MASK depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
};

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
    const PsoSet& GetPso(const std::string& name, BlendMode blendMode = BlendMode::None, FillMode fillMode = FillMode::kSolid);

    D3D12_STATIC_SAMPLER_DESC StaticSamplers();

private:
    PSOManager() = default;
    ~PSOManager() = default;

    void CreatePso(const std::string& name, BlendMode blend, FillMode fill);
    D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode);
    void EnsureShaders(const std::string& name, Microsoft::WRL::ComPtr<IDxcBlob>& outVS, Microsoft::WRL::ComPtr<IDxcBlob>& outPS);

    struct CacheKey {
        std::string name;
        BlendMode blend;
        FillMode fill;
        bool operator==(const CacheKey& o) const {
            return name == o.name && blend == o.blend && fill == o.fill;
        }
    };

    struct KeyHasher {
        std::size_t operator()(const CacheKey& k) const {
            // 全ての要素をハッシュ計算に含める
            size_t h1 = std::hash<std::string>()(k.name);
            size_t h2 = std::hash<int>()((int)k.blend);
            size_t h3 = std::hash<int>()((int)k.fill);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    static std::unique_ptr<PSOManager> instance_;
    std::unordered_map<std::string, PsoConfig> psoConfigs_;
    std::unordered_map<CacheKey, PsoSet, KeyHasher> psoCache_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSigCache_;
    std::unordered_map<std::string, ShaderSet> shaderCache_;
};