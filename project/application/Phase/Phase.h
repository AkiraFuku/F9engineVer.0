#pragma once
class Scene;

class Phase
{
public:

    virtual ~Phase() = default;  // ← 仮想デストラクタを追加

    virtual  void Initialize(Scene* scene){};
    virtual  void Update(Scene* scene) {};
    virtual  void Draw(Scene* scene) {};
    virtual  void Finalize(Scene* scene) {};

};

