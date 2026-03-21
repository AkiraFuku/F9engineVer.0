#include "WinApp.h"
#include "assert.h"
#ifdef USE_IMGUI
#include"imgui_impl_win32.h"
#include"imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI


// 静的メンバ変数の定義
std::unique_ptr<WinApp> WinApp::instance = nullptr;

WinApp* WinApp::GetInstance() {
    if (instance == nullptr) {
        // std::make_uniqueはプライベートコンストラクタにアクセスできないため、newを使用
        instance.reset(new WinApp);
    }
    return instance.get();
}

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI


    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    {
        return true;
    }
#endif // USE_IMGUI
    switch (msg) {
        //
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize() {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);


    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラスの名前
    wc.lpszClassName = L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //ウィンドウクラスの登録
    RegisterClass(&wc);
    // Output


    //OutputDebugStringA("Hello,DirectX!\n");


    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    //  
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

    //ウィンドウの作成
    hwnd = CreateWindow(
        wc.lpszClassName,//クラス名
        L"CG2",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );


    //ウィンドウを表示
    ShowWindow(hwnd, SW_SHOW);
    //

    timeBeginPeriod(1);



};
void WinApp::Update() {
}
void WinApp::Finalize()
{
    CoUninitialize();
    //デバッグレイヤーの解放

    CloseWindow(hwnd);

    ///デバッグレイヤーのライブオブジェクトのレポート

}
bool WinApp::ProcessMessage()
{
    MSG msg{};
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT)
    {
        return true;
    }

    return false;
}
;