#include "Framework.h"
#include "ParticleManager.h"//フレームワークに移植
#include "SceneManager.h"
#include "WinApp.h"//フレームワークに移植
#include "ImGuiManager.h"//フレームワークに移植
#include"Audio.h"//フレームワークに移植
#include"Input.h"//フレームワークに移植
#include"StringUtility.h"//フレームワークに移植
#include"Logger.h"//フレームワークに移植
#include "SpriteCommon.h"//フレームワークに移植
#include "TextureManager.h"//フレームワークに移植
#include"Object3DCommon.h"//フレームワークに移植
#include "ModelCommon.h"//フレームワークに移植
#include "ModelManager.h"//フレームワークに移植
#include "SrvManager.h"//フレームワークに移植
#include <fstream>
#include <iostream> 
#include "PSOMnager.h"
#include "LightManager.h"
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
    //ダンプファイルの作成
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t filePath[MAX_PATH] = { 0 };
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH,
        L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
        time.wYear, time.wMonth, time.wDay,
        time.wHour, time.wMinute);
    HANDLE dumpFileHandle = CreateFile(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0, CREATE_ALWAYS, 0, 0
    );
    //プロセスIDとクラッシュが発生したスレッドIDを取得
    DWORD procesessId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    //設定情報入力
    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
    minidumpInformation.ThreadId = threadId;
    minidumpInformation.ExceptionPointers = exception;
    minidumpInformation.ClientPointers = TRUE;
    //ダンプの出力
    MiniDumpWriteDump(
        GetCurrentProcess(),
        procesessId,
        dumpFileHandle,
        MiniDumpNormal,
        &minidumpInformation,
        nullptr,
        nullptr
    );
    return EXCEPTION_EXECUTE_HANDLER;
}

std::wstring wstr = L"Hello,DirectX!";
void Framework::Initialize()
{
    //D3D12の初期化
    CoInitializeEx(0, COINIT_MULTITHREADED);

    SetUnhandledExceptionFilter(ExportDump);
    //ログ出力用のディレクトリを作成
    std::filesystem::create_directory("logs");
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
        nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::chrono::zoned_time localTime{
        std::chrono::current_zone(),
        nowSeconds
    };
    std::string dataString = std::format(
        "{:%Y%m%d_%H%M%S}",
        localTime
    );
    std::string logFilePath = std::string("logs/") + dataString + ".log";
    //ファイルへの書き込み
    std::ofstream logStream(logFilePath);

    
    WinApp::GetInstance()->Initialize();

  
    // 引数には生のポインタが必要なので .get() を使用
    DXCommon::GetInstance()->Initialize();
 
    SrvManager::GetInstance()->Initialize();
PSOMnager::GetInstance()->Initialize();

  

    ImGuiManager::GetInstance()->Initialize();
    TextureManager::GetInstance()->Initialize();
    ModelManager::GetInstance()->Initialize();
    ParticleManager::GetInstance()->Initialize();
    Logger::Log(StringUtility::ConvertString(std::format(L"WSTRING{}\n", wstr)));
    LightManager::GetInstance()->Initialize();
    // 外部入力
    Input::GetInstance()->Initialize();

    SpriteCommon::GetInstance()->Initialize();
    Object3dCommon::GetInstance()->Initialize();
    Audio::GetInstance()->Initialize();


}

void Framework::Finalize()
{
    LightManager::GetInstance()->Finalize();
    SrvManager::GetInstance()->Finalize();
    SceneManager::GetInstance()->Finalize();
    DXCommon::GetInstance()->Finalize();
    PSOMnager::GetInstance()->Finalize();
    Audio::GetInstance()->Finalize();
    Input::GetInstance()->Finalize();
    Object3dCommon::GetInstance()->Finalize();

    ImGuiManager::GetInstance()->Finalize();
   
    SpriteCommon::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();

    WinApp::GetInstance()->Finalize();
}
void Framework::Update()
{
    //メッセージがある限りGetMessageを呼び出す
    if (WinApp::GetInstance()->ProcessMessage()) {
        endRequest_ = true;


    }
#ifdef USE_IMGUI
    ImGuiManager::GetInstance()->Begin();
#endif
    Input::GetInstance()->Update();
    Audio::GetInstance()-> Update();
    LightManager::GetInstance()->Update();
    //DXCommon::GetInstance()->PreDraw();
    //SrvManager::GetInstance()->PreDraw();
  
}

void Framework::Draw()
{
  /*  ImGuiManager::GetInstance()->End();
    ImGuiManager::GetInstance()->Draw();
    DXCommon::GetInstance()->PostDraw();
    TextureManager::GetInstance()->ReleaseIntermediateResources();*/

    // 基底クラスの描画処理（純粋仮想関数として宣言されているため、必ずオーバーライドする必要があります）
}
void Framework::Run()
{
    Initialize();
    while (true) {
        Update();
        if (IsEnd()) {
            break;
        }
        Draw();
    }
    Finalize();
}
