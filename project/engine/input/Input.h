#pragma once
#include <Windows.h>
#include "wrl.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#include <memory>
class Input
{
public:
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    // シングルトン化に伴う追記
    static Input* GetInstance();
    friend struct std::default_delete<Input>;
    void Finalize();

    // マウスのボタン定義
    enum  MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2,
    };
    struct MoveMouse
    {
        LONG x;
        LONG y;
        LONG z;
    };
public:
    void Initialize();

    void Update();
    // --- キーボード用 ---
    bool PushedKeyDown(BYTE keys);
    bool PushedKeyUp(BYTE keys);
    bool TriggerKeyDown(BYTE keys);
    bool TriggerKeyUp(BYTE keys);

    // --- マウス用 ---
    /// <summary>
    /// マウスボタンが押されているか
    /// </summary>
    bool PushMouseDown(int32_t button);
    /// <summary>
    /// マウスボタンが押されないか
    /// </summary>
    bool PushMouseUP(int32_t button);

    /// <summary>
    /// マウスボタンがトリガーされたか（押した瞬間）
    /// </summary>
    bool TriggerMouseDown(int32_t button);
    /// <summary>
    /// マウスボタンがトリガーされたか（離した瞬間）
    /// </summary>
    bool TriggerMouseUP(int32_t button);

    /// <summary>
    /// マウスの移動量を取得
    /// </summary>
    MoveMouse GetMouseMove();

    //　ゲームパッド用もここに追加予定
    bool GetJoyStick(int32_t stickNo, XINPUT_STATE& out) const;
    bool GetPreJoyStick(int32_t stickNo, XINPUT_STATE& out) const;
    void SetDeadZone(int32_t stickNo, int32_t deadZoneL, int32_t deadZoneR);

    size_t GetConnectedStickNum();



    /// <summary>

   /// <summary>
    /// padのボタンが押されているか (Press)
    /// </summary>
    bool PushPadDown(int32_t stickNo, WORD button) const;

    /// <summary>
    /// padのボタンが押されていないか (Release state)
    /// </summary>
    bool PushPadUP(int32_t stickNo, WORD button) const;

    /// <summary>
    /// padのボタンがトリガーされたか（押した瞬間 / Trigger）
    /// </summary>
    bool TriggerPadDown(int32_t stickNo, WORD button) const;

    /// <summary>
    /// padのボタンが離されたか（離した瞬間 / Release trigger）
    /// </summary>
    bool TriggerPadUp(int32_t stickNo, WORD button) const;

      static std::unique_ptr<Input> instance;
private:

    // シングルトンパターン: コンストラクタ等を隠蔽
  
    Input() = default;
    ~Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    ComPtr<IDirectInputDevice8> keyboard;
    ComPtr<IDirectInputDevice8> mouse;
    BYTE preKey[256] = {};
    BYTE key[256] = {};


    // --- マウス用変数 ---
    // マウスの状態を格納する構造体
    DIMOUSESTATE mouseState = {};
    DIMOUSESTATE preMouseState = {};
  

    // --- ゲームパッド用変数 ---
    // メンバ変数として保持する
    int deadZoneL_[XUSER_MAX_COUNT];
    int deadZoneR_[XUSER_MAX_COUNT];

    XINPUT_STATE state_[XUSER_MAX_COUNT];    // 現在の状態
    XINPUT_STATE preState_[XUSER_MAX_COUNT]; // 1フレーム前の状態
};

