#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
class SceneManager;
class Camera;
class  Phase;

class Scene
{
public:
    virtual ~Scene() = default;
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual void SetSceneManager(SceneManager* sceneManager) {
        sceneManager_ = sceneManager;
    }
    SceneManager* GetSceneManager() {
        return sceneManager_;
    }

    void ChangeActiveCamera(Camera* targetCamera);

    Camera* GetActiveCamera() const {
        return activeCamera_;
    }

   void ChangePhase(std::unique_ptr<Phase> nextPhase);



private:
    SceneManager* sceneManager_ = nullptr;

protected:
    uint32_t BGMHandle_ = 0;
    Camera* activeCamera_ = nullptr;
    std::map<std::string, std::unique_ptr<Camera>> cameraMap_;

    std::unique_ptr<Phase> currentPhase_;

};

