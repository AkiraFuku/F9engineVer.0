#include "ParticleManager.h"
#include "Logger.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "MathFunction.h"
#include <numbers>
#include "PSOMnager.h"
#pragma once
std::unique_ptr<ParticleManager>  ParticleManager::instance;
uint32_t ParticleManager::kMaxNumInstance = 1024;
void ParticleManager::Initialize() {
    //DXCommonとSRVマネージャーの受け取り
    //ランダムエンジンの初期化
    randomEngine_.seed(seedGen_());
    //パイプラインステート生成


    PsoProperty pso = { PipelineType::Particle,BlendMode::Add };
    PsoSet psoset = PSOMnager::GetInstance()->GetPsoSet(pso);
    graphicsPipelineState_ = psoset.pipelineState;
    rootSignature_ = psoset.rootSignature;

    //  CreatePSO();
      //頂点データの初期化（座標等）
      //頂点リソース生成
      //頂点バッファビュー（VBV）を作成
      //頂点リソースにデータを書き込む
    CreateVertexBuffer();
    CreateMaterialBuffer();
}
ParticleManager* ParticleManager::GetInstance() {
    if (instance == nullptr)
    {
        // privateコンストラクタのため reset(new ...) を使用
        instance.reset(new ParticleManager());
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
            (*particleIterator).transfom.translate += (*particleIterator).velocity * DXCommon::kDeltaTime;
            (*particleIterator).currentTime += DXCommon::kDeltaTime;

            if (numInstance < kMaxNumInstance)
            {

                particleGroup.instancingData[numInstance].color.w = alpha;
                Matrix4x4 worldMatrix = {};
                /*  if (isBillboard)
                  {*/
                //(*particleIterator).transfom.rotate.z += 1.0f / 60.0f;


                worldMatrix = MakeBillboardMatrix((*particleIterator).transfom.scale, (*particleIterator).transfom.rotate, billboardMatrix, (*particleIterator).transfom.translate);

                particleGroup.instancingData[numInstance].WVP = Multiply(worldMatrix, viewProjectionMatrix);
                particleGroup.instancingData[numInstance].color.x = (*particleIterator).color.x;
                particleGroup.instancingData[numInstance].color.y = (*particleIterator).color.y;
                particleGroup.instancingData[numInstance].color.z = (*particleIterator).color.z;
                ++numInstance;
            }
            ++particleIterator;

        }
        particleGroup.kNumInstance = numInstance;
    }
}
void ParticleManager::Draw() {
    // RootSignatureの設定
    DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    //PSOの設定
    DXCommon::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
    DXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //VBVの設定
    DXCommon::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

    for (auto& [key, particleGroup] : particleGroups) {
        if (particleGroup.kNumInstance > 0) {

            // とりあえずコードの意図を汲んで修正すると：
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());

            // [1] Descriptor Table (Instancing Data): インスタンシング用SRV
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(particleGroup.instancingSrvIndex));

            // [2] Descriptor Table (Texture): テクスチャ用SRV
            DXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(particleGroup.materialData.textureIndex));
            // DrawCall
            // 後述するトポロジーの修正に合わせて頂点数を変更 (6 -> 4)
            DXCommon::GetInstance()->GetCommandList()->DrawInstanced(4, particleGroup.kNumInstance, 0, 0);
        }
    }
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilepath)
{
    assert(!particleGroups.contains(name));
    //
    ParticleGroup& newParticle = particleGroups[name];
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
        newParticle.instancingData[i].WVP = Makeidetity4x4(); // 単位行列などで埋める
        newParticle.instancingData[i].color = { 1.0f, 1.0f, 1.0f, 0.0f };
    }
}

void ParticleManager::Emit(const std::string name, const Vector3& postion, uint32_t count)
{
    assert(particleGroups.contains(name));

    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distTime(1.0f, 10.0f);
    std::uniform_real_distribution<float> distScale(0.4f, 1.5f);

    for (uint32_t i = 0; i < count; ++i)
    {
        Particle particle;
        particle.transfom.scale = { 0.05f,distScale(randomEngine_),1.0f };
        particle.transfom.rotate = { 0.0f,0.0f, distribution(randomEngine_) };
        Vector3 randamTranslate = { distribution(randomEngine_),distribution(randomEngine_) ,distribution(randomEngine_) };
        particle.transfom.translate = postion ;//+ randamTranslate;
        particle.velocity ={0.0f,0.0f,0.0f};// { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };

        particle.color ={1.0f,1.0f,1.0f,1.0f};// { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_),1.0f };

        particle.lifeTime = 1.0f;//distTime(randomEngine_);
        particle.currentTime = 0.0f;
        particleGroups[name].particles.push_back(particle);
    }

}

void ParticleManager::CreateRootSignature()
{
    PsoProperty pso = { PipelineType::Particle,BlendMode::Add };
    PsoSet psoset = PSOMnager::GetInstance()->GetPsoSet(pso);
    graphicsPipelineState_ = psoset.pipelineState;
    rootSignature_ = psoset.rootSignature;
}
void ParticleManager::CreateVertexBuffer() {
    VertexData vertices[] = {
        // Position(x,y,z,w)             TexCoord(u,v)   Normal(x,y,z)
        {{-1.0f,  1.0f, 0.0f, 1.0f},     {0.0f, 0.0f},   {0.0f, 0.0f, 1.0f}}, // 左上
        {{ 1.0f,  1.0f, 0.0f, 1.0f},     {1.0f, 0.0f},   {0.0f, 0.0f, 1.0f}}, // 右上
        {{-1.0f, -1.0f, 0.0f, 1.0f},     {0.0f, 1.0f},   {0.0f, 0.0f, 1.0f}}, // 左下
        {{ 1.0f, -1.0f, 0.0f, 1.0f},     {1.0f, 1.0f},   {0.0f, 0.0f, 1.0f}}, // 右下
    };
    size_t sizeIB = sizeof(vertices);
    //頂点リソースの作成
    vertexResourse_ =
        DXCommon::GetInstance()->
        CreateBufferResource(sizeIB);
    //頂点バッファビューの設定
    vertexBufferView_.BufferLocation =
        vertexResourse_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = UINT(sizeIB);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    vertexResourse_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    //頂点データの転送
    memcpy(vertexData_, vertices, sizeIB);

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
    materialData_->uvTransform = Makeidetity4x4();
}
