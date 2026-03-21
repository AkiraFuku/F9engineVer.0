#include "ImGuiManager.h"
#include "SrvManager.h"
#include "WinApp.h"
#include "DXCommon.h"

// 静的メンバ変数の実体
std::unique_ptr<ImGuiManager> ImGuiManager::instance = nullptr;

ImGuiManager* ImGuiManager::GetInstance() {
    if (instance == nullptr) {
        // コンストラクタがprivateなので reset(new ...) を使用
        instance.reset(new ImGuiManager());
    }
    return instance.get();
}

void ImGuiManager::Initialize() {
    #ifdef USE_IMGUI

   
   
  

  

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = "externals/imgui/my_imgui_settings.ini";
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());
    uint32_t fontSrvIndex =
        SrvManager::GetInstance()->AllocateSRV(); // フォント用SRVを確保

    // descriptorHeap_=;
    ImGui_ImplDX12_Init(
        DXCommon::GetInstance()->GetDevice().Get(),
        static_cast<int>(DXCommon::GetInstance()->GetSwapChainBufferCount()),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        SrvManager::GetInstance()->GetDescriptorHeap().Get(),
        SrvManager::GetInstance()->GetCPUDescriptorHandle(fontSrvIndex),
        SrvManager::GetInstance()->GetGPUDescriptorHandle(fontSrvIndex)

    );
#endif // USE_IMGUI

}
void ImGuiManager::Finalize() {
    #ifdef USE_IMGUI

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif // USE_IMGUI

}
void ImGuiManager::Begin() {
    #ifdef USE_IMGUI

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif // USE_IMGUI

}
void ImGuiManager::End() {
    #ifdef USE_IMGUI

    ImGui::Render();
#endif // USE_IMGUI

}
void ImGuiManager::Draw() {
    #ifdef USE_IMGUI

    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList().Get();

    ID3D12DescriptorHeap* ppHeaps[] = { SrvManager::GetInstance()->GetDescriptorHeap().Get() };
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // USE_IMGUI

}