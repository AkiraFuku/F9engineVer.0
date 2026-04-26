#include "InputHandler.h"
#include "Input.h"
std::vector<std::unique_ptr<ICommand>> InputHandler::HandleInput()
{
    std::vector<std::unique_ptr<ICommand>> commands;
    Input* input = Input::GetInstance();
    // 1. スティック移動の処理
    XINPUT_STATE state;
    if (input->GetJoyStick(0, state) || (input->PushedKeyDown(DIK_A) || input->PushedKeyDown(DIK_D)))
    {
        if (input->GetJoyStick(0, state)) {
            float rawX = (float)state.Gamepad.sThumbLX / 32767.0f;
            if (std::abs(rawX) > 0.2f) {
                commands.push_back(std::make_unique<MoveCommand>(rawX));
            }
        }
        //　キー入力の例（WASDで移動、スペースでジャンプなど）もここに追加可
        if (input->PushedKeyDown(DIK_A) || input->PushedKeyDown(DIK_D))
        {
            if (input->PushedKeyDown(DIK_A))
            {
                commands.push_back(std::make_unique<MoveCommand>(-1.0f)); // 左移動
            }
            if (input->PushedKeyDown(DIK_D))
            {
                commands.push_back(std::make_unique<MoveCommand>(1.0f)); // 右移動
            }
        }
    }

    if (input->TriggerPadDown(0, XINPUT_GAMEPAD_A) || input->TriggerKeyDown(DIK_SPACE))
    {
        // 2. ジャンプの処理 (TriggerPadDownを使用)
        if (input->TriggerPadDown(0, XINPUT_GAMEPAD_A)) {
            commands.push_back(std::make_unique<JumpCommand>());
        }
        if (input->TriggerKeyDown(DIK_SPACE))
        {
            commands.push_back(std::make_unique<JumpCommand>());
        }
    }

    if (input->TriggerMouseDown(0)) { // 例としてスペース
        commands.push_back(std::make_unique<AttackCommand>());
    }

    return commands;
}
