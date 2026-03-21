#include "PSOManager.h" // ファイル名変更に合わせてインクルードも変更
#include "DXCommon.h"
#include "Logger.h"
#include <cassert>
#include <d3d12.h>
#include <dxcapi.h> // IDxcBlobのため


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

std::unique_ptr<PSOManager> PSOManager::instance_ = nullptr;

PSOManager* PSOManager::GetInstance() {
    if (!instance_) {
        instance_.reset(new PSOManager());
    }
    return instance_.get();
}



void PSOManager::Initialize() {
    psoCache_.clear();
    rootSigCache_.clear();
    psoConfigs_.clear();
}

void PSOManager::Finalize() {
    psoCache_.clear();
    rootSigCache_.clear();
    psoConfigs_.clear();
}

void PSOManager::RegisterPsoGenerator(const std::string& name, const PsoConfig& psoConfig) {
    psoConfigs_[name] = psoConfig;
}

D3D12_STATIC_SAMPLER_DESC PSOManager::StaticSamplers()
{

    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    return sampler;
}

const PsoSet& PSOManager::GetPso(const std::string& name, BlendMode blend, FillMode fill) {
    CacheKey key{ name, blend, fill };
    if (psoCache_.contains(key)) {
        return psoCache_[key];
    }
    CreatePso(name, blend, fill);
    return psoCache_.at(key);
}


// -------------------------------------------------------------------------
// シェーダー管理（重複コンパイル防止）
// -------------------------------------------------------------------------
// 修正内容: contains()にはキー（name）を渡す必要がある
void PSOManager::EnsureShaders(const std::string& name, Microsoft::WRL::ComPtr<IDxcBlob>& outVS, Microsoft::WRL::ComPtr<IDxcBlob>& outPS) {
    // 既にキャッシュにあればそれを返す
    if (shaderCache_.contains(name)) {
        outVS = shaderCache_[name].vs;
        outPS = shaderCache_[name].ps;
        return;
    }

    // 新規コンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vs = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob> ps = nullptr;

    auto dxCommon = DXCommon::GetInstance();

    psoConfigs_[name];

    vs = dxCommon->CompileShader(psoConfigs_[name].vsPath, L"vs_6_0");
    ps = dxCommon->CompileShader(psoConfigs_[name].psPath, L"ps_6_0");



    assert(vs && ps);

    // キャッシュに保存
    shaderCache_[name] = { vs, ps };

    outVS = vs;
    outPS = ps;


}


// -------------------------------------------------------------------------
// PSO 生成
// -------------------------------------------------------------------------
 void PSOManager::CreatePso(const std::string& name, BlendMode blend, FillMode fill) {
    auto device = DXCommon::GetInstance()->GetDevice();
    const auto& config = psoConfigs_.at(name);

    // 1. RootSignature のキャッシュ確認と生成
    if (!rootSigCache_.contains(name)) {
        assert(config.rootSignatureGenerator && "RootSignatureGenerator is null");
        rootSigCache_[name] = config.rootSignatureGenerator();
    }
    auto rootSignature = rootSigCache_[name];

    // 2. InputLayout の取得
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    if (config.inputLayoutGenerator) {
        inputElements = config.inputLayoutGenerator();
    }

    // 3. Shader の取得
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, psBlob;
    EnsureShaders(name, vsBlob, psBlob);

    // 4. PSO構築
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();

    // InputLayout
    psoDesc.InputLayout = { inputElements.data(), static_cast<UINT>(inputElements.size()) };

    // Shaders
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

    // Blend State
    psoDesc.BlendState = CreateBlendDesc(blend);

    // Rasterizer State
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FillMode = (fill == FillMode::kWireFrame) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = config.cullMode;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    // DepthStencil State (Configからの値を優先)
    psoDesc.DepthStencilState = config.depth;
    psoDesc.DepthStencilState.DepthEnable = config.depthEnable;
    psoDesc.DepthStencilState.DepthWriteMask = config.depthWriteMask;
    // 深度比較関数が設定されていない場合のデフォルト
    if(psoDesc.DepthStencilState.DepthFunc == 0) {
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    }

    // フォーマット設定 (環境に合わせて適宜変更)
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.NumRenderTargets = 1;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    PsoSet psoSet;
    psoSet.rootSignature = rootSignature;
    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoSet.pipelineState));
    assert(SUCCEEDED(hr) && "Failed to create Pipeline State");

    psoCache_[{name, blend, fill}] = psoSet;
}

D3D12_BLEND_DESC PSOManager::CreateBlendDesc(BlendMode mode) {
    D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;

    // 共通初期値
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    switch (mode) {
    case BlendMode::None:
        blendDesc.RenderTarget[0].BlendEnable = FALSE;
        break;
    case BlendMode::Normal:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Add:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Subtract:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        break;
    case BlendMode::Multiply:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    case BlendMode::Screen:
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        break;
    }
    return blendDesc;
}