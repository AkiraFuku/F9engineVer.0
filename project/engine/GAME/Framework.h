#pragma once
#include<Windows.h>
#include<filesystem>
#include<strsafe.h>
#include<dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")
#include <memory>
#include"D3DResourceLeakChecker.h"//フレームワークに移植


#include "DXCommon.h"//フレームワークに移植

#include "AbstractSceneFactory.h"
class Framework
{
public:
    virtual ~Framework() = default;
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw() = 0;
    bool IsEnd() {
        return endReqest_;
    }
    void Run();

    void RequestEnd() {
        endReqest_ = true;
    }

    AbstractSceneFactory* GetSceneFactory() {
        return sceneFactory;
    }

private:
    bool endReqest_ = false;

    std::unique_ptr<D3DResourceLeakChecker> LeakCheck;  // スタック変数から動的割り当てに変更
    
    /*std::unique_ptr<DXCommon> dxCommon;*/
  

    AbstractSceneFactory* sceneFactory = nullptr;
};

