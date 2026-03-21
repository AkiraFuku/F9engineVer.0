#include "SpriteCommon.h"
#include "Logger.h"
#include <cassert>
#include "DXCommon.h"
#include "PSOManager.h"
// 静的メンバ変数の初期化
std::unique_ptr<SpriteCommon> SpriteCommon::instance = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {
    if (instance == nullptr) {
        instance.reset(new SpriteCommon());
    }
    return instance.get();
}

void SpriteCommon::Finalize() {

    instance.reset();
}
void SpriteCommon::Initialize()
{

    PsoConfig config{};
    config.vsPath = L"resources/shaders/Sprite/Sprite.vs.hlsl";
    config.psPath = L"resources/shaders/Sprite/Sprite.ps.hlsl";


    config.rootSignatureGenerator = []() {
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler = PSOManager::GetInstance()->StaticSamplers();

        staticSamplers.push_back(sampler);
        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        rootParameters.resize(3);

        // Enum定義 (可読性のため)
        enum {
            kMaterial, kTransform, kTexture,
        };
        // 0. Material (CBV b0, Pixel)
        rootParameters[kMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kMaterial].Descriptor.ShaderRegister = 0;
        rootParameters[kMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // 1. Transform (CBV b0, Vertex)
        rootParameters[kTransform].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kTransform].Descriptor.ShaderRegister = 1;
        rootParameters[kTransform].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // 2. Texture (Table t0, Pixel)
        rootParameters[kTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[kTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kTexture].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[kTexture].DescriptorTable.NumDescriptorRanges = 1;

        // シリアライズ
        D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
        descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        descriptionRootSignature.pParameters = rootParameters.data();
        descriptionRootSignature.NumParameters = (UINT)rootParameters.size();
        descriptionRootSignature.pStaticSamplers = staticSamplers.data();
        descriptionRootSignature.NumStaticSamplers = (UINT)staticSamplers.size();


        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));



        return rootSignature;
        };
    config.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };
    // 深度設定
    config.depthEnable = true;
    config.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    // PSOManagerに名前を付けて登録
    PSOManager::GetInstance()->RegisterPsoGenerator("Sprite", config);
    auto psoSet = PSOManager::GetInstance()->GetPso("Sprite");
    rootSignature_ = psoSet.rootSignature;
    graphicsPipelineState_ = psoSet.pipelineState;
    //CreatePSO();

}

void SpriteCommon::SpriteCommonDraw()
{
    // RootSignatureの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    //  //PSOの設定
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());

    //プリミティブトポロジーのセット
    DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

