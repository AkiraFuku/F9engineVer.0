#pragma once

#include "Scene.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "Framework.h"
#include "SceneFactory.h"
#include <memory>
class GameEngine : public Framework
{
public:
    void Initialize() override;

    void Finalize()override;

    void Update()override;
    void PreDraw()override;

    void Draw()override;


private:


    std::unique_ptr <SceneFactory> sceneFactory_;

private:
    //ログファイルパス
    const std::filesystem::path logFilePath = "D3DResourceLeakLog.txt";

    

};

