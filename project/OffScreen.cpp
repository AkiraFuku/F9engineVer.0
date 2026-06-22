#include "OffScreen.h"
#include "DXCommon.h"
#include "PSOManager.h"
#include "Logger.h"
#include "SrvManager.h"
#include <cassert>
#include "Camera.h"
#include "mathfunction.h"
#include "TextureManager.h"
// インスタンス定義
std::unique_ptr<OffScreen> OffScreen::instance = nullptr;

OffScreen* OffScreen::GetInstance() {
    if (instance == nullptr) {
        struct Helper : public OffScreen {
            Helper() : OffScreen() {}
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();
}

void OffScreen::Initialize() {


    MaskMaterial_.textureFilePath = "resources/Mask/noise0.png";
    TextureManager::GetInstance()->LoadTexture(MaskMaterial_.textureFilePath);
    MaskMaterial_.maskTextureSrvIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(MaskMaterial_.textureFilePath);

    PsoConfig psoConfig = {};
    psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/CopyImage/CopyImage.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での初期化
        HRESULT hr;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler = PSOManager::GetInstance()->StaticSamplers();
        staticSamplers.push_back(sampler);

        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        CD3DX12_ROOT_PARAMETER rootParameters[1]{};
        rootParameters[0].InitAsDescriptorTable(1, &descRangeTexture[0]);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        // hr = ... (ここでは auto を付けない)
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));
        return rootSignature;
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("OffScreen", psoConfig);
    psoConfig = {};

    psoConfig.shaderPaths.clear();  psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/OutLine/LuminanceBasedOutline.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での初期化
        HRESULT hr;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler = PSOManager::GetInstance()->StaticSamplers();
        staticSamplers.push_back(sampler);

        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        CD3DX12_ROOT_PARAMETER rootParameters[1]{};
        rootParameters[0].InitAsDescriptorTable(1, &descRangeTexture[0]);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        // hr = ... (ここでは auto を付けない)
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));
        return rootSignature;
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("Vignette", psoConfig);
    //ぶらー
    psoConfig = {};

    psoConfig.shaderPaths.clear();  psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/RadialBlur/RadialBlur.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での初期化
        HRESULT hr;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler = PSOManager::GetInstance()->StaticSamplers();
        staticSamplers.push_back(sampler);

        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        CD3DX12_ROOT_PARAMETER rootParameters[2]{};
        rootParameters[0].InitAsDescriptorTable(1, &descRangeTexture[0]);
        rootParameters[1].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        // hr = ... (ここでは auto を付けない)
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));
        return rootSignature;
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("RadialBlur", psoConfig);
    psoConfig = {};

    psoConfig.shaderPaths.clear();  psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/OutLine/DepthBasedOutline.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での初期化
        HRESULT hr;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler = PSOManager::GetInstance()->StaticSamplers();
        staticSamplers.push_back(sampler);

        sampler.ShaderRegister = 1; // 1番目のルートパラメータ用
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // ポイントフィルタに変更
        staticSamplers.push_back(sampler);

        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // 深度SRV
        D3D12_DESCRIPTOR_RANGE descRangeDepth[1]{};
        descRangeDepth[0].BaseShaderRegister = 1;
        descRangeDepth[0].NumDescriptors = 1;
        descRangeDepth[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeDepth[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        CD3DX12_ROOT_PARAMETER rootParameters[3]{};
        rootParameters[0].InitAsDescriptorTable(1, &descRangeTexture[0]);
        rootParameters[1].InitAsDescriptorTable(1, &descRangeDepth[0]);
        rootParameters[2].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        // hr = ... (ここでは auto を付けない)
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));
        return rootSignature;
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("DepthOutline", psoConfig);
    //ディゾルブ
    psoConfig = {};

    psoConfig.shaderPaths.clear();  psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/Dissolve/Dissolve.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での初期化
        HRESULT hr;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler = PSOManager::GetInstance()->StaticSamplers();
        staticSamplers.push_back(sampler);

       

        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0;
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // 2. マスクテクスチャ用の範囲 (t1) ★追加
        D3D12_DESCRIPTOR_RANGE descRangeMask[1]{};
        descRangeMask[0].BaseShaderRegister = 1;
        descRangeMask[0].NumDescriptors = 1;
        descRangeMask[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeMask[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        CD3DX12_ROOT_PARAMETER rootParameters[3]{};
        rootParameters[0].InitAsDescriptorTable(1, &descRangeTexture[0]);
        rootParameters[1].InitAsDescriptorTable(1, &descRangeMask[0]);
        rootParameters[2].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        // hr = ... (ここでは auto を付けない)
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));
        return rootSignature;
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("Dissolve", psoConfig);

    // マテリアル定数バッファの生成とマップ
    materialConstantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
    materialConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));


    // Initializeの末尾などで生成（materialConstantBuffer_ と同様の処理）
    blurConstantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(BlurParam));
    blurConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&blurParamData_));

    // 初期値をセットしておく
    blurParamData_->center = Vector2(0.5f, 0.5f);
    blurParamData_->radius = 16; // HLSL側の int samples に対応
    blurParamData_->blurWidth = 0.01f;

    // ディゾルブ定数バッファの生成とマップ
    dissolveConstantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(DissolveParm));
    dissolveConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dissolveParamData_));


    dissolveParamData_->threshold = 0.5f; // 初期値
}

void OffScreen::Draw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    auto psoManager = PSOManager::GetInstance();
    auto srvManager = SrvManager::GetInstance();
    if (!camera_)
    {

        Logger::Log("Camera is not set for OffScreen.");
        return;

    }
    materialData_->projectionInverse = Inverse(
        camera_->GetProjectionMatrix()
    );

    // 1. PSOの取得とセット
    PsoSet pso = psoManager->GetPso("Dissolve");
    commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
    commandList->SetPipelineState(pso.pipelineState.Get());

    // 2. トポロジの設定（これを忘れると何も出ません）
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 3. SRVのセット（第1引数はルートパラメータの番号: 今回は0番）
    uint32_t srvIndex = DXCommon::GetInstance()->GetRenderTextureSrvIndex();
    srvManager->SetGraphicsRootDescriptorTable(0, srvIndex);


    srvManager->SetGraphicsRootDescriptorTable(1, MaskMaterial_.maskTextureSrvIndex);

    //commandList->SetGraphicsRootConstantBufferView(1, blurConstantBuffer_->GetGPUVirtualAddress());
    // srvIndex = DXCommon::GetInstance()->GetDepthTextureSrvIndex();
    //srvManager->SetGraphicsRootDescriptorTable(1, srvIndex);

    //commandList->SetGraphicsRootConstantBufferView(2, materialConstantBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(2, dissolveConstantBuffer_->GetGPUVirtualAddress());

    // 4. 描画実行（頂点シェーダーで全画面生成している場合は3頂点）
    commandList->DrawInstanced(3, 1, 0, 0);
}

void OffScreen::SetMaskMaterial(std::string textureFilePath)
{}
