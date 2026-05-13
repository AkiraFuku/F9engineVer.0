#include "Object3dCommon.h"
#include "Logger.h"
#include "PSOManager.h"

// 静的メンバ変数の初期化
std::unique_ptr<Object3dCommon> Object3dCommon::instance = nullptr;
Object3dCommon* Object3dCommon::GetInstance() {
    if (instance == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public Object3dCommon {
            Helper() : Object3dCommon() {
            }
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();
}

void Object3dCommon::Finalize() {
    instance.reset(); // 解放
}
void Object3dCommon::Initialize()
{
    PsoConfig config{};

    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Object3d/Object3d.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Object3d/Object3d.ps.hlsl", "main", L"ps_6_0" };



    config.shaderPaths.push_back(vsPath);
    config.shaderPaths.push_back(psPath);

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

        D3D12_DESCRIPTOR_RANGE descRangeEnv[1]{};
        descRangeEnv[0].BaseShaderRegister = 4; // t1 (通常のテクスチャがt0の場合)
        descRangeEnv[0].NumDescriptors = 1;
        descRangeEnv[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeEnv[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        rootParameters.resize(9);



        // Enum定義 (可読性のため)
        enum {
            kMaterial, kTransform, kTexture, DirLight, PointLight, SpotLight, Count, kCamera, kEnviroment
        };

        // 0. Material (CBV b0, Pixel)
        rootParameters[kMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kMaterial].Descriptor.ShaderRegister = 0;

        // 1. Transform (CBV b0, Vertex)
        rootParameters[kTransform].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kTransform].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[kTransform].Descriptor.ShaderRegister = 0;

        // 2. Texture (Table t0, Pixel)
        rootParameters[kTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[kTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kTexture].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[kTexture].DescriptorTable.NumDescriptorRanges = 1;

        // ★変更: 3. DirectionalLight (SRV t1)
        rootParameters[DirLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[DirLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[DirLight].Descriptor.ShaderRegister = 1; // t1

        // ★追加: 4. PointLight (SRV t2)
        rootParameters[PointLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[PointLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[PointLight].Descriptor.ShaderRegister = 2; // t2

        // ★追加: 5. SpotLight (SRV t3)
        rootParameters[SpotLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[SpotLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[SpotLight].Descriptor.ShaderRegister = 3; // t3

        // ★追加: 6. LightCounts (CBV b3)
        rootParameters[Count].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[Count].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[Count].Descriptor.ShaderRegister = 3; // b3 (b0,b1,b2は使用済みと仮定、あるいは空いている番号)
        //7カメラ
        rootParameters[kCamera].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 定数バッファビュー
        rootParameters[kCamera].Descriptor.ShaderRegister = 2; // レジスタ番号 2 (b2)
        rootParameters[kCamera].Descriptor.RegisterSpace = 0;
        rootParameters[kCamera].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーのみ見える

        // 8. kEnviromentTexture (Table t0, Pixel)
        rootParameters[kEnviroment].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[kEnviroment].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kEnviroment].DescriptorTable.pDescriptorRanges = descRangeEnv;
        rootParameters[kEnviroment].DescriptorTable.NumDescriptorRanges = 1;

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
        InputLayout inputLayout = {};

        inputLayout.inputElement = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        
        inputLayout.inputLayout.pInputElementDescs = inputLayout.inputElement.data();
        inputLayout.inputLayout.NumElements = static_cast<UINT>(inputLayout.inputElement.size());
        return inputLayout;
        };
    // 深度設定
    config.depthEnable = true;
    config.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    // PSOManagerに名前を付けて登録
    PSOManager::GetInstance()->RegisterPsoGenerator("Object3d", config);
    auto psoSet = PSOManager::GetInstance()->GetPso("Object3d");
    rootSignature_ = psoSet.rootSignature;
    graphicsPipelineState_ = psoSet.pipelineState;

    //   CreatePSO();
}
void Object3dCommon::Object3dCommonDraw()
{
    // RootSignatureの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    //  //PSOの設定
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
    //プリミティブトポロジーのセット
    DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::CreateRootSignature()
{
    ///ディスクプリプターレンジの作成
    D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
    descriptorRange[0].BaseShaderRegister = 0;//シェーダーのレジスタ番号0
    descriptorRange[0].NumDescriptors = 1;//ディスクリプタの数1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//テーブルの先頭からオフセットなし
    ///

    // RootSignatureの作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatur{};
    descriptionRootSignatur.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ///ルートパラメータの設定
    D3D12_ROOT_PARAMETER rootParameters[4]{};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;//シェーダーのレジスタ番号0とバインド
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//ヴァーテックスシェーダーで使う
    rootParameters[1].Descriptor.ShaderRegister = 0;//シェーダーのレジスタ番号0とバインド
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//ディスクリプタテーブルを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//ディスクリプタレンジの設定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//ディスクリプタレンジの数
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使う
    rootParameters[3].Descriptor.ShaderRegister = 1;

    descriptionRootSignatur.pParameters = rootParameters;//ルートパラメータの設定
    descriptionRootSignatur.NumParameters = _countof(rootParameters);//ルートパラメータの数

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//線形フィルタリング
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//テクスチャのアドレスモードはラップ
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較関数は使用しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//最大LODは最大値
    staticSamplers[0].ShaderRegister = 0;//シェーダーのレジスタ番号0
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用する
    descriptionRootSignatur.pStaticSamplers = staticSamplers;//スタティックサンプラーの設定
    descriptionRootSignatur.NumStaticSamplers = _countof(staticSamplers);//スタティックサンプラーの数

    //シリアライズしてバイナリにする;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr< ID3DBlob> errorBlob = nullptr;
    hr_ = D3D12SerializeRootSignature(
        &descriptionRootSignatur,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );
    if (FAILED(hr_)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元にルートシグネチャー生成
    //ID3D12RootSignature* rootSignature = nullptr;
    hr_ = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(
        0,
        signatureBlob.Get()->GetBufferPointer(),
        signatureBlob.Get()->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );
    assert(SUCCEEDED(hr_));
}
void Object3dCommon::CreatePSO() {
    CreateRootSignature();
    //InputLayoutの設定
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    //
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    //
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    //RasteriwrStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;//カリングなし
    //BACK;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    //shaderのコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DXCommon::GetInstance()->CompileShader(
        L"resources/shaders/Object3d/Object3D.vs.hlsl",
        L"vs_6_0"


    );
    assert(vertexShaderBlob.Get() != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DXCommon::GetInstance()->CompileShader(
        L"resources/shaders/Object3d/Object3D.ps.hlsl",
        L"ps_6_0"

    );
    assert(pixelShaderBlob.Get() != nullptr);


    //DSSの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;//深度テストを有効にする
    //書き込み
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    //PSOの生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
    graphicPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
    vertexShaderBlob->GetBufferSize() };//ヴァーテックスシェーダー
    graphicPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
    pixelShaderBlob->GetBufferSize() };//ピクセルシェーダー
    graphicPipelineStateDesc.BlendState = blendDesc;//ブレンドステート
    graphicPipelineStateDesc.RasterizerState = rasterizerDesc;//ラスタライザーステート
    ///巻き込むRTV
    graphicPipelineStateDesc.NumRenderTargets = 1;
    graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    ///トポロジー
    graphicPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //カラー
    graphicPipelineStateDesc.SampleDesc.Count = 1;
    graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    //深度ステンシルビューの設定
    graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;//PSOにDSSを設定
    graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルビューのフォーマット
    ////
    //PSOの生成
    hr_ = DXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(
        &graphicPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState_)
    );
    assert(SUCCEEDED(hr_));
}