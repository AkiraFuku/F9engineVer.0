#include "ParticleManager.h"
#include "Logger.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "MathFunction.h"
#include <numbers>
#include "PSOManager.h"
#pragma once
std::unique_ptr<ParticleManager>  ParticleManager::instance;
uint32_t ParticleManager::kMaxNumInstance = 1024;
void ParticleManager::Initialize() {
    //DXCommonとSRVマネージャーの受け取り
    //ランダムエンジンの初期化
    randomEngine_.seed(seedGen_());
    //パイプラインステート生成







    PsoConfig configCylinder{};

    configCylinder.shaderPaths.push_back(PsoConfig::ShaderPath{ ShaderType::VS, L"resources/shaders/Cylinder/ParticleCylinder.vs.hlsl", "main", L"vs_6_0" });
    configCylinder.shaderPaths.push_back(PsoConfig::ShaderPath{ ShaderType::PS, L"resources/shaders/Cylinder/ParticleCylinder.ps.hlsl", "main", L"ps_6_0" });






    configCylinder.rootSignatureGenerator = []() {
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler = PSOManager::GetInstance()->StaticSamplers();

        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        staticSamplers.push_back(sampler);
        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        // パーティクル用インスタンシングレンジ (VS t0)
            // ※ staticにしないとスコープを抜けてデータが壊れる可能性があるため注意
            //   ここでは関数内完結させるため、vector等で管理するか、static配列にする
        static D3D12_DESCRIPTOR_RANGE descRangeInstancing[1]{};
        descRangeInstancing[0].BaseShaderRegister = 0; // t0 (VS)
        descRangeInstancing[0].NumDescriptors = 1;
        descRangeInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rootParameters.resize(4);

        // [Param 0] Material (CBV b0, Pixel)
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].Descriptor.ShaderRegister = 0;

        // [Param 1] Instancing Data (DescriptorTable t0, Vertex) ★ここが重要
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[1].DescriptorTable.pDescriptorRanges = descRangeInstancing;
        rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

        // [Param 2] Texture (DescriptorTable t0, Pixel)
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[2].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

        // [Param 3] DirectionalLight (CBV b1, Pixel)
        rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[3].Descriptor.ShaderRegister = 1;
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
    configCylinder.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };
    // 深度設定
    configCylinder.depthEnable = true;
    configCylinder.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    configCylinder.cullMode = D3D12_CULL_MODE_NONE; // パーティクルは両面描画することが多いので、カリングなしに設定

    // PSOManagerに名前を付けて登録
    PSOManager::GetInstance()->RegisterPsoGenerator("ParticleCylinder", configCylinder);
    //auto psoSet = PSOManager::GetInstance()->GetPso("Particle", BlendMode::Normal);
    PsoConfig config{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Particle/Particle.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Particle/Particle.ps.hlsl", "main", L"ps_6_0" };
    config.shaderPaths.push_back(vsPath);
    config.shaderPaths.push_back(psPath);






    config.rootSignatureGenerator = []() {
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler = PSOManager::GetInstance()->StaticSamplers();

        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        staticSamplers.push_back(sampler);
        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


        // パーティクル用インスタンシングレンジ (VS t0)
            // ※ staticにしないとスコープを抜けてデータが壊れる可能性があるため注意
            //   ここでは関数内完結させるため、vector等で管理するか、static配列にする
        static D3D12_DESCRIPTOR_RANGE descRangeInstancing[1]{};
        descRangeInstancing[0].BaseShaderRegister = 0; // t0 (VS)
        descRangeInstancing[0].NumDescriptors = 1;
        descRangeInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rootParameters.resize(4);

        // [Param 0] Material (CBV b0, Pixel)
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].Descriptor.ShaderRegister = 0;

        // [Param 1] Instancing Data (DescriptorTable t0, Vertex) ★ここが重要
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[1].DescriptorTable.pDescriptorRanges = descRangeInstancing;
        rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

        // [Param 2] Texture (DescriptorTable t0, Pixel)
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[2].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

        // [Param 3] DirectionalLight (CBV b1, Pixel)
        rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[3].Descriptor.ShaderRegister = 1;
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
    config.cullMode = D3D12_CULL_MODE_NONE; // パーティクルは両面描画することが多いので、カリングなしに設定

    // PSOManagerに名前を付けて登録
    PSOManager::GetInstance()->RegisterPsoGenerator("Particle", config);
    /*    auto psoSet = PSOManager::GetInstance()->GetPso("Particle", BlendMode::Normal);
       /* graphicsPipelineState_ = psoSet.pipelineState;
        rootSignature_ = psoSet.rootSignature;*/

        //  CreatePSO();
          //頂点データの初期化（座標等）
          //頂点リソース生成
          //頂点バッファビュー（VBV）を作成
          //頂点リソースにデータを書き込む
        // 全形状のバッファを作成
    CreateVertexBuffer(EffectType::Plane);
    CreateVertexBuffer(EffectType::Ring);
    CreateVertexBuffer(EffectType::Cylinder);
    CreateMaterialBuffer();
}
ParticleManager* ParticleManager::GetInstance() {
    if (instance == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public ParticleManager {
            Helper() : ParticleManager() {
            }
        };
        instance = std::make_unique<Helper>();
    }
    return instance.get();

};
void ParticleManager::Finalize() {

    // インスタンスの破棄
    instance.reset();
}

void ParticleManager::ReleaseParticleGroup(const std::string name)
{
    particleGroups.erase(name);
}



void ParticleManager::Update() {
    Matrix4x4 backFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
    //ビルボード行列計算
    Matrix4x4 billboardMatrix = Multiply(backFrontMatrix, camera_->GetWorldMatrix());
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;
    //カメラからビューとプロジェクション行列
    Matrix4x4 viewMatrix = camera_->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
    Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
    //
    for (auto& [key, particleGroup] : particleGroups)
    {
        uint32_t  numInstance = 0;
        for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin();
            particleIterator != particleGroup.particles.end();
            )
        {




            if ((*particleIterator).lifeTime <= (*particleIterator).currentTime)
            {
                particleIterator = particleGroup.particles.erase(particleIterator);
                continue;
            }

            float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
            (*particleIterator).transform.translate += (*particleIterator).velocity * DXCommon::kDeltaTime;
            (*particleIterator).currentTime += DXCommon::kDeltaTime;

            if (numInstance < kMaxNumInstance)
            {

                particleGroup.instancingData[numInstance].color.w = alpha;
                Matrix4x4 worldMatrix = {};

                if (particleGroup.update) {
                    particleGroup.update(*particleIterator, DXCommon::kDeltaTime);
                }

                if (particleGroup.effectType == EffectType::Plane)
                {
                    worldMatrix = MakeBillboardMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, billboardMatrix, (*particleIterator).transform.translate);

                } else
                {
                    worldMatrix = MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);

                }

                particleGroup.instancingData[numInstance].WVP = Multiply(worldMatrix, viewProjectionMatrix);
                /*   particleGroup.instancingData[numInstance].color.x = (*particleIterator).color.x;
                   particleGroup.instancingData[numInstance].color.y = (*particleIterator).color.y;
                   particleGroup.instancingData[numInstance].color.z = (*particleIterator).color.z;*/
                particleGroup.instancingData[numInstance].color = (*particleIterator).color;
                Matrix4x4 uvMatrix = MakeAffineMatrix(
                    Vector3{ particleIterator->uvTransform.scale.x, particleIterator->uvTransform.scale.y, 1.0f }, // Scale
                    Vector3{ 0.0f, 0.0f, particleIterator->uvTransform.rotate },         // Rotate
                    Vector3{ particleIterator->uvTransform.offset.x, particleIterator->uvTransform.offset.y, 0.0f } // Translate
                );
                particleGroup.instancingData[numInstance].uvTransform = uvMatrix;
                ++numInstance;
            }
            ++particleIterator;

        }
        particleGroup.kNumInstance = numInstance;
    }
}
void ParticleManager::Draw() {
    PsoSet psoSet{};

    //VBVの設定
    //DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

    for (auto& [key, particleGroup] : particleGroups) {
        if (particleGroup.kNumInstance > 0) {

            const auto& primitive = primitiveResources_[particleGroup.effectType];
            if (particleGroup.effectType == EffectType::Cylinder)
            {
                psoSet = PSOManager::GetInstance()->GetPso("ParticleCylinder", BlendMode::Normal);
            } else
            {
                psoSet = PSOManager::GetInstance()->GetPso("Particle", BlendMode::Add);
            }

            // RootSignatureの設定
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(psoSet.rootSignature.Get());
            //PSOの設定
            DXCommon::GetInstance()->GetCommandList()->SetPipelineState(psoSet.pipelineState.Get());
            DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &primitive.vbv);


            // とりあえずコードの意図を汲んで修正すると：
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());

            // [1] Descriptor Table (Instancing Data): インスタンシング用SRV
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(particleGroup.instancingSrvIndex));

            // [2] Descriptor Table (Texture): テクスチャ用SRV
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(particleGroup.materialData.textureIndex));
            // DrawCall
            // 後述するトポロジーの修正に合わせて頂点数を変更 (6 -> 4)
            DXCommon::GetInstance()->GetCommandList()->DrawInstanced(primitive.vertexCount, particleGroup.kNumInstance, 0, 0);
        }
    }
}

void ParticleManager::CreateParticleGroup(
    const std::string name,
    const std::string textureFilepath,
    EffectType type,
    ParticleEmitterFunc initialize,
    ParticleUpdateFunc update
)
{

    assert(!particleGroups.contains(name));
    //
    ParticleGroup& newParticle = particleGroups[name];
    newParticle.initialize = initialize;
    newParticle.update = update;
    newParticle.name = name;
    newParticle.effectType = type; // ★形状を保存
    newParticle.materialData.textureFilePath = textureFilepath;
    newParticle.kNumInstance = kMaxNumInstance;
    newParticle.materialData.textureIndex = newParticle.materialData.textureIndex =
        TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilepath);
    newParticle.instancingResource = DXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * newParticle.kNumInstance);

    newParticle.instancingSrvIndex = SrvManager::GetInstance()->AllocateSRV();

    SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
        newParticle.instancingSrvIndex,
        newParticle.instancingResource.Get(),
        newParticle.kNumInstance,
        sizeof(ParticleForGPU)
    );
    newParticle.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&newParticle.instancingData));
    for (uint32_t i = 0; i < newParticle.kNumInstance; ++i) {
        newParticle.instancingData[i].WVP = Makeidentity4x4(); // 単位行列などで埋める
        newParticle.instancingData[i].color = { 1.0f, 1.0f, 1.0f, 0.0f };
        newParticle.instancingData[i].uvTransform = Makeidentity4x4(); // 単位行列などで埋める
    }
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count)
{
    assert(particleGroups.contains(name));

    auto& group = particleGroups[name];
    for (uint32_t i = 0; i < count; ++i) {
        //Particle p;
        if (group.initialize) {
            // 関数が「値」を返すようになったので、そのまま push_back できる
            group.particles.push_back(group.initialize(position, randomEngine_));
        } else {
            group.particles.push_back(MakeParticle(randomEngine_, position));
        }
    }

    /*    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
        std::uniform_real_distribution<float> distTime(1.0f, 10.0f);

        for (uint32_t i = 0; i < count; ++i)
        {
            Particle particle;
            Vector3 randomTranslate = { distribution(randomEngine_),distribution(randomEngine_) ,distribution(randomEngine_) };

            Vector3 pPosition = position;
            particle = MakeParticle(randomEngine_, pPosition);

            particleGroups[name].particles.push_back(particle);
        }*/

}
std::vector<ParticleManager::VertexData> ParticleManager::PrimitiveVertexPlane()
{

    return {
         {{-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
         {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    };
}
std::vector<ParticleManager::VertexData> ParticleManager::PrimitiveVertexRing()
{
    const uint32_t kRingDivide = 32;
    const float kOuterRadius = 1.0f;
    const float kInnerRadius = 0.2f;
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);
    std::vector<ParticleManager::VertexData> vertices;
    // 1セグメントにつき2つの三角形（3頂点×2）を作るため、あらかじめ予約
    vertices.reserve(kRingDivide * 6);
    for (uint32_t i = 0; i < kRingDivide; ++i)
    {
        float s = std::sin(i * radianPerDivide);
        float c = std::cos(i * radianPerDivide);
        float sNext = std::sin((i + 1) * radianPerDivide);
        float cNext = std::cos((i + 1) * radianPerDivide);
        float u = float(i) / float(kRingDivide);
        float uNext = float(i + 1) / float(kRingDivide);

        // 4つの角の頂点座標を計算
        // p1: 内側(現在), p2: 外側(現在), p3: 内側(次), p4: 外側(次)
        VertexData p1 = { {c * kInnerRadius, s * kInnerRadius, 0.0f,1.0f}, {u, 1.0f} }; // 内
        VertexData p2 = { {c * kOuterRadius, s * kOuterRadius, 0.0f,1.0f}, {u, 0.0f} }; // 外
        VertexData p3 = { {cNext * kInnerRadius, sNext * kInnerRadius, 0.0f,1.0f}, {uNext, 1.0f} }; // 内(次)
        VertexData p4 = { {cNext * kOuterRadius, sNext * kOuterRadius, 0.0f,1.0f}, {uNext, 0.0f} }; // 外(次)

        // 三角形1: p1 -> p2 -> p3
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);

        // 三角形2: p2 -> p4 -> p3
        vertices.push_back(p2);
        vertices.push_back(p4);
        vertices.push_back(p3);
    }


    return vertices;
}
std::vector<ParticleManager::VertexData> ParticleManager::PrimitiveVertexCylinder()
{
    const uint32_t kDivide = 32;
    const float kTopRadius = 0.5f;
    const float kBottomRadius = 0.5f;
    const float kHeight = 1.0f;
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kDivide);
    std::vector<ParticleManager::VertexData> vertices;

    for (uint32_t i = 0; i < kDivide; ++i)
    {
        float s = std::sin(i * radianPerDivide);
        float c = std::cos(i * radianPerDivide);
        float sNext = std::sin((i + 1) * radianPerDivide);
        float cNext = std::cos((i + 1) * radianPerDivide);
        float u = float(i) / float(kDivide);
        float uNext = float(i + 1) / float(kDivide);

        // 4つの角の頂点座標を計算
        // p1: 内側(現在), p2: 外側(現在), p3: 内側(次), p4: 外側(次)
        VertexData p1 = { {-s * kTopRadius, kHeight,c * kTopRadius,1.0f}, {u, 0.0f}, {-s, 0.0f, c} }; // 内
        VertexData p2 = { {-sNext * kTopRadius,kHeight,cNext * kTopRadius,1.0f }, {uNext, 0.0f}, {-sNext, 0.0f, cNext} }; // 外
        VertexData p3 = { {-s * kBottomRadius, 0.0f,c * kBottomRadius,1.0f}, {u, 1.0f}, {-s, 0.0f, c} }; // 内(次)
        VertexData p4 = { {-sNext * kBottomRadius,0.0f,cNext * kBottomRadius,1.0f }, {uNext, 1.0f}, {-sNext, 0.0f, cNext} }; // 外(次)

        // 三角形1: p1 -> p2 -> p3
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);

        // 三角形2: p2 -> p4 -> p3
        vertices.push_back(p2);
        vertices.push_back(p4);
        vertices.push_back(p3);
    }



    return vertices;
}
ParticleManager::Particle ParticleManager::MakeParticle(std::mt19937& randomEngine, const Vector3& translate)
{
    std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> distScale(0.4f, 1.5f);
    Particle particle;
    // particle.transform.scale = { 1.0f,distScale(randomEngine),1.0f };
    particle.transform.scale = { 1.0f,1.0f,1.0f };
    //  particle.transform.rotate = { 0.0f,0.0f,distRotate(randomEngine) };
    particle.transform.rotate = { 0.0f,0.0f,0.0f };
    particle.transform.translate = translate;
    particle.velocity = { 0.0f,0.0f,0.0f };

    particle.color = { 1.0f,1.0f,1.0f,1.0f };

    particle.lifeTime = 1.0f;
    particle.currentTime = 0.0f;

    return particle;
}
void ParticleManager::CreateVertexBuffer(EffectType type) {
    std::vector<VertexData> vertices;
    switch (type) {
    case EffectType::Plane:    vertices = PrimitiveVertexPlane(); break;
    case EffectType::Ring:     vertices = PrimitiveVertexRing(); break;
    case EffectType::Cylinder: vertices = PrimitiveVertexCylinder(); break;
    }

    size_t sizeVB = sizeof(VertexData) * vertices.size();
    PrimitiveResource res;
    res.resource = DXCommon::GetInstance()->CreateBufferResource(sizeVB);
    res.vertexCount = static_cast<uint32_t>(vertices.size());

    res.vbv.BufferLocation = res.resource->GetGPUVirtualAddress();
    res.vbv.SizeInBytes = static_cast<UINT>(sizeVB);
    res.vbv.StrideInBytes = sizeof(VertexData);

    void* mappedData = nullptr;
    res.resource->Map(0, nullptr, &mappedData);
    memcpy(mappedData, vertices.data(), sizeVB);
    res.resource->Unmap(0, nullptr);

    primitiveResources_[type] = std::move(res);
}
void ParticleManager::CreateMaterialBuffer()
{
    //データの設定

    materialResource_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeof(Material));
    materialResource_->
        Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
    materialData_->uvTransform = Makeidentity4x4();
}
