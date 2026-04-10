#pragma once
class SceneManager;
class Camera;
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
    SceneManager*  GetSceneManager() {
        return sceneManager_;
    }
private:
    SceneManager* sceneManager_ = nullptr;

protected:
    uint32_t BGMHandle_ = 0;
  std::unique_ptr<Camera>curuntCamera = nullptr;
};

