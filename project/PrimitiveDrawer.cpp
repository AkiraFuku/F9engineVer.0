#include "PrimitiveDrawer.h"

#include "DXCommon.h"
#include "PSOManager.h"
#include "Logger.h"
#include <cassert>

std::unique_ptr<PrimitiveDrawer> PrimitiveDrawer::instance_ = nullptr;


PrimitiveDrawer* PrimitiveDrawer::GetInstance() {
    if (instance_ == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public PrimitiveDrawer {
            Helper() : PrimitiveDrawer() {
            }
        };
        instance_ = std::make_unique<Helper>();
    }
    return instance_.get();
}

void PrimitiveDrawer::Finalize() {
}



void PrimitiveDrawer::AddPSO()
{
    PsoConfig config{};
    PsoConfig::ShaderPath vsPath{ ShaderType::VS, L"resources/shaders/Primitive/Primitive.vs.hlsl", "main", L"vs_6_0" };
    PsoConfig::ShaderPath psPath{ ShaderType::PS, L"resources/shaders/Primitive/Primitive.ps.hlsl", "main", L"ps_6_0" };

    config.shaderPaths.push_back(vsPath);
    config.shaderPaths.push_back(psPath);

    config.rootSignatureGenerator = []() {

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            // エラー処理
           // return Microsoft::WRL::ComPtr<ID3D12RootSignature>();

            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr)) {
            // エラー処理
            Logger::Log("Failed to create root signature for PrimitiveDrawer.");
            assert(false);
        }

        return rootSignature;


        };

    config.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };

    PSOManager::GetInstance()->RegisterPsoGenerator("Primitive", config);

}

void PrimitiveDrawer::Initialize() {
    AddPSO();

    vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(
        sizeof(VertexData) * 3
    );
    
    vertexBufferView_.BufferLocation = vertexResource_.Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;  // 修正：4 → 3
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
    vertexData_[0].position = Vector4(0.0f, 0.5f, 0.0f, 1.0f);
    vertexData_[1].position = Vector4(0.5f, -0.5f, 0.0f, 1.0f);
    vertexData_[2].position = Vector4(-0.5f, -0.5f, 0.0f, 1.0f);
    vertexResource_->Unmap(0, nullptr);  // 追加：Unmapを呼び出す
}

void PrimitiveDrawer::Draw() {
    PsoSet psoSet =
        PSOManager::GetInstance()->GetPso("Primitive", BlendMode::None, FillMode::kSolid, Toporogy::TriangleList);
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());
    commandList->SetPipelineState(psoSet.pipelineState.Get());



    // 描画コマンドの発行

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0); // 仮に3頂点を描画
}
