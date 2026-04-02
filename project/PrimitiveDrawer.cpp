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



        D3D12_ROOT_PARAMETER rootParameter[1]{};
        rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameter[0].Descriptor.ShaderRegister = 0; // b0

        rootSignatureDesc.pParameters = rootParameter;
        rootSignatureDesc.NumParameters = _countof(rootParameter);




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
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };

    PSOManager::GetInstance()->RegisterPsoGenerator("Primitive", config);

}

void PrimitiveDrawer::Initialize() {
    AddPSO();

   vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(
    sizeof(VertexData) * kMaxVertices
);
vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
vertexBufferView_.SizeInBytes = sizeof(VertexData) * kMaxVertices;
vertexBufferView_.StrideInBytes = sizeof(VertexData);


}

void PrimitiveDrawer::Draw() {
    if (vertices_.empty()) return;

    // 1. GPU上のバッファへ現在の頂点リストをコピー
    void* mappedPtr = nullptr;
    vertexResource_->Map(0, nullptr, &mappedPtr);
    std::memcpy(mappedPtr, vertices_.data(), sizeof(VertexData) * vertices_.size());
    vertexResource_->Unmap(0, nullptr);

    // 2. コマンドの発行
    auto commandList = DXCommon::GetInstance()->GetCommandList();


    PsoSet psoSet = PSOManager::GetInstance()->GetPso("Primitive", BlendMode::Normal, FillMode::kSolid, Toporogy::LineList);

    commandList->SetPipelineState(psoSet.pipelineState.Get());
    commandList->SetGraphicsRootSignature(psoSet.rootSignature.Get());
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); // ライン描画の場合
    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);

    // 3. 次のフレームのためにリストをクリア
    vertices_.clear();
}
void PrimitiveDrawer::DrawLine(const Vector3& p1, const Vector3& p2, const Vector4& color) {
    if (vertices_.size() + 2 > kMaxVertices) return; // 溢れ防止

    // 始点
    vertices_.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
    // 終点
    vertices_.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });

    //this->Draw();
}