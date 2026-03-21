#pragma once
#include <Windows.h>
#include<cstdint>
#pragma comment(lib,"winmm.lib")
#include <memory>
class WinApp
{

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    // シングルトンインスタンスの取得
    static WinApp* GetInstance();
public:
     friend struct std::default_delete<WinApp>;
    // コピーコンストラクタと代入演算子を禁止（シングルトン保証）
    WinApp(const WinApp&) = delete;
    WinApp& operator=(const WinApp&) = delete;
    //初期化
    void Initialize();
    //更新
    void Update();
    //終了
    void Finalize();

    // //ウィンドウのサイズ
    static const int32_t kClientWidth = 1280;
    static const int32_t kClientHeight = 720;

    HWND GetHwnd()const {
        return hwnd;
    }
    HINSTANCE GetInstanceHandle()const {
        return wc.hInstance;
    }
    bool ProcessMessage();
private: // プライベートメソッド
    // コンストラクタとデストラクタを隠蔽
    WinApp() = default;
    ~WinApp() = default;

private: // メンバ変数
    // シングルトンインスタンス
    static std::unique_ptr<WinApp> instance;
    HWND hwnd = nullptr;
    WNDCLASS wc{};
    //メッセージ

};

