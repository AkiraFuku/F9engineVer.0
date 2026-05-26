#pragma once
class Scene;

class Phase
{
public:
  virtual  void Initialize(Scene* scene )=0;
  virtual  void Update(Scene* scene )=0;
  virtual  void Draw(Scene* scene )=0;
  virtual  void Finalize(Scene* scene )=0;

};

