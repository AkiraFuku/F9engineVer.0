#pragma once

#include"Scene.h"
#include <string>
#include <memory>
class AbstractSceneFactory{

    public:
    virtual ~AbstractSceneFactory() = default;
   virtual std::unique_ptr<Scene> CreateScene(const std::string& sceneName) = 0;
};