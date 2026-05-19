#pragma once
#include <wrl.h>
#include <memory>
class ImGuiManager
{
public:

    // インスタンス取得
    static ImGuiManager* GetInstance();
    // スマートポインタ用
    static std::unique_ptr<ImGuiManager> instance;
     friend struct std::default_delete<ImGuiManager>;
    void Initialize();
    void Finalize();
    void Begin();
    void End();
    void Draw();
private:
    // シングルトン化のためコンストラクタ類をprivateにする
    ImGuiManager() = default;
    ~ImGuiManager() = default;
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

    
    
   
};

