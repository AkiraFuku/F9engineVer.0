#pragma once

#pragma once
#include <memory>
#include <vector>
#include "Command.h"

class InputHandler
{public:
    // 毎フレームの入力をチェックし、実行すべきコマンドのリストを返す
    std::vector<std::unique_ptr<ICommand>> HandleInput();
};
