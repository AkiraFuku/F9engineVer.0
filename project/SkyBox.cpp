#include "SkyBox.h"
#include "Logger.h"
#include "PSOManager.h"
#include "TextureManager.h"
#include "DXCommon.h"
#include "MathFunction.h"

void SkyBox::Initialize()
{

    PsoConfig PSO{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Skybox/Skybox.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Skybox/Skybox.ps.hlsl", "main", L"ps_6_0" };
    PSO.shaderPaths.push_back(vsPath);
    PSO.shaderPaths.push_back(psPath);

    PSO.rootSignatureGenerator = []() {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        D3D12_ROOT_PARAMETER rootParameter[3]{};
        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameter[0].Descriptor.ShaderRegister = 0; // b0
        // 1. Transform (CBV b0, Vertex)
        rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameter[1].Descriptor.ShaderRegister = 1; // b1
        // 2. Texture (Table t0, Pixel)
        rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameter[2].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
        rootSignatureDesc.pParameters = rootParameter;
        rootSignatureDesc.NumParameters = _countof(rootParameter);

        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler = PSOManager::GetInstance()->StaticSamplers();

        staticSamplers.push_back(sampler);

        // シリアライズ
        D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
        descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        descriptionRootSignature.pParameters = rootParameter;
        descriptionRootSignature.NumParameters = _countof(rootParameter);
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

    PSO.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        };
    PSO.cullMode = D3D12_CULL_MODE_FRONT;
    PSO.depthEnable = true;
    PSO.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    PSOManager::GetInstance()->RegisterPsoGenerator("SkyBox", PSO);

    vertexData_ = new VertexData[24];
    //右 描画インデックス[0,1,2],[2,1,3]
    vertexData_[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertexData_[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
    vertexData_[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };
    //左　描画インデックス[4,5,6],[6,5,7]
    vertexData_[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertexData_[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
    vertexData_[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };
    //前 描画インデックス[8,9,10],[10,9,11]
    vertexData_[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertexData_[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertexData_[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
    vertexData_[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };
    //後ろ 描画インデックス[12,13,14],[14,13,15]
    vertexData_[12].position = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[13].position = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[14].position = { -1.0f, -1.0f, -1.0f, 1.0f };
    vertexData_[15].position = { 1.0f, -1.0f, -1.0f, 1.0f };
    //上 描画インデックス[16,17,18],[18,17,19]
    vertexData_[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[17].position = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertexData_[18].position = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertexData_[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };
    //下 描画インデックス[20,21,22],[22,21,23]
    vertexData_[20].position = { -1.0f, -1.0f, 1.0f, 1.0f };
    vertexData_[21].position = { 1.0f, -1.0f, 1.0f, 1.0f };
    vertexData_[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
    vertexData_[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };

    vertexResourse_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(VertexData) * 24);
    vertexBufferView_.BufferLocation = vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 24;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    materialData_ = new Material;
    materialResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    //座標変換
    transformationMatrixResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_.Get()->
        Map(0, nullptr, reinterpret_cast<void**>(&wvpResource_));
    wvpResource_->WVP = Makeidetity4x4();
    wvpResource_->World = Makeidetity4x4();
    wvpResource_->WorldInverseTranspose = Inverse(wvpResource_->World);


    transform_.scale = Vector3(1.0f, 1.0f, 1.0f);
    transform_.rotate = Vector3(0.0f, 0.0f, 0.0f);
    transform_.translate = Vector3(0.0f, 0.0f, 0.0f);



}

void SkyBox::Finalize()
{
}

void SkyBox::Update()
{
    Matrix4x4 worldMatrix = MakeAfineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 worldViewProjectionMatrix = {};
    //ワールド行列とビュー行列とプロジェクション行列を掛け算
    if (camera_)
    {
        // cameraData_->worldPosition = camera_->GetTranslate();
        worldViewProjectionMatrix = Multiply(worldMatrix, camera_->GetViewProtectionMatrix());
        //   worldViewProjectionMatrix = Multiply( worldMatrix, camera_->GetViewProtectionMatrix());
    } else {
        worldViewProjectionMatrix = Multiply(worldMatrix, Makeidetity4x4());
    }
    //行列をGPUに転送
    wvpResource_->WVP = worldViewProjectionMatrix;
    wvpResource_->World = worldMatrix;
    wvpResource_->WorldInverseTranspose = Transpose(Inverse(worldMatrix));
}

void SkyBox::Draw()
{
    auto psoSet = PSOManager::GetInstance()->GetPso("SkyBox", BlendMode::Normal, FillMode::kSolid);
    // PSOをセット
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
    //パイプラインステートとルートシグネチャの設定

    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(psoSet.rootSignature.Get());
    DXCommon::GetInstance()->
        GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    //インデックスバッファの設定
    //マテリアルの設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // SRVのディスクプリプターテーブルの先頭を設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
    //座標変換行列の設定
    DXCommon::GetInstance()->
        GetCommandList()->
        SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());


    DXCommon::GetInstance()->GetCommandList()->DrawInstanced(24, 1, 0, 0);
}

void SkyBox::SetTextureByFilePath(const std::string& textureFilePath)
{
    textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

}
