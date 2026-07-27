#include "OffScreen.h"
#include "DXCommon.h"
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
        // ラムダ式内での記述
        return RootSignatureBuilder()
            // [Param 0] Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // スタティックサンプラー追加
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("OffScreen", psoConfig);
    //ランダム
    psoConfig = {};
    psoConfig.shaderPaths.clear();
    psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/Random/Random.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        return RootSignatureBuilder()
            // [Param 0] Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 1] Material/Transform (CBV b0, ALL)
            .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)

            // スタティックサンプラー追加
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
        };

    psoConfig.inputLayoutGenerator = []() {


        InputLayout inputLayout = {};
        inputLayout.inputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };;
        return inputLayout;

        };
    psoConfig.depthEnable = false;

    PSOManager::GetInstance()->RegisterPsoGenerator("random", psoConfig);
    psoConfig = {};

    psoConfig.shaderPaths.clear();  psoConfig.shaderPaths = {
        { ShaderType::VS, L"resources/shaders/CopyImage/FullScreen.vs.hlsl", "main", L"vs_6_0" },
        { ShaderType::PS, L"resources/shaders/OutLine/LuminanceBasedOutline.ps.hlsl", "main", L"ps_6_0" }
    };

    psoConfig.rootSignatureGenerator = []() {
        // ラムダ式内での記述
        return RootSignatureBuilder()
            // [Param 0] Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // スタティックサンプラー追加
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
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
        // ラムダ式内での記述
        return RootSignatureBuilder()
            // スタティックサンプラー追加
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())
            // [Param 0] Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 1] CBV (b0, ALL)
            .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)

 

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
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
        // ラムダ式内での記述
        auto sampler0 = PSOManager::GetInstance()->StaticSamplers(); // s0 (リニアフィルタ等)

        auto sampler1 = sampler0;
        sampler1.ShaderRegister = 1;                        // s1
        sampler1.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;   // ポイントフィルタ

        return RootSignatureBuilder()
            // [Param 0] Color Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 1] Depth Texture (DescriptorTable t1, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 2] CBV (b0, ALL)
            .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)

            // スタティックサンプラー追加 (s0, s1)
            .AddStaticSampler(sampler0)
            .AddStaticSampler(sampler1)

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
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
        // ラムダ式内での記述
        return RootSignatureBuilder()
            // [Param 0] Main Texture (DescriptorTable t0, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 1] Mask Texture (DescriptorTable t1, ALL)
            .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_ALL)

            // [Param 2] CBV (b0, ALL)
            .AddCBV(0, D3D12_SHADER_VISIBILITY_ALL)

            // スタティックサンプラー追加
            .AddStaticSampler(PSOManager::GetInstance()->StaticSamplers())

            // ビルド＆生成
            .Build(DXCommon::GetInstance()->GetDevice().Get());
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

    //if (materialData_)
    //{
    //    // 毎フレームの経過時間を加算する
    //    materialData_->time += DXCommon::GetInstance()->kDeltaTime;
    //}

     materialData_->projectionInverse = Inverse(
         camera_->GetProjectionMatrix()
     );

     // 1. PSOの取得とセット
    PsoSet pso = psoManager->GetPso("DepthOutline");
    commandList->SetGraphicsRootSignature(pso.rootSignature.Get());
    commandList->SetPipelineState(pso.pipelineState.Get());

    // 2. トポロジの設定（これを忘れると何も出ません）
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 3. SRVのセット（第1引数はルートパラメータの番号: 今回は0番）
    uint32_t srvIndex = DXCommon::GetInstance()->GetRenderTextureSrvIndex();
    srvManager->SetGraphicsRootDescriptorTable(0, srvIndex);

    //
    //srvManager->SetGraphicsRootDescriptorTable(1, MaskMaterial_.maskTextureSrvIndex);

    //commandList->SetGraphicsRootConstantBufferView(1, blurConstantBuffer_->GetGPUVirtualAddress());
     srvIndex = DXCommon::GetInstance()->GetDepthTextureSrvIndex();
    srvManager->SetGraphicsRootDescriptorTable(1, srvIndex);

   // commandList->SetGraphicsRootConstantBufferView(1, materialConstantBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(2, materialConstantBuffer_->GetGPUVirtualAddress());
    //commandList->SetGraphicsRootConstantBufferView(2, dissolveConstantBuffer_->GetGPUVirtualAddress());

    // 4. 描画実行（頂点シェーダーで全画面生成している場合は3頂点）
    commandList->DrawInstanced(3, 1, 0, 0);
}

void OffScreen::SetMaskMaterial(std::string textureFilePath)
{}
