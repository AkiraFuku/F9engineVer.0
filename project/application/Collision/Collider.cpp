#include "Collider.h"

#include"GameObject.h"


#include "PrimitiveDrawer.h"

void Collider::initialize(GameObject* owner, float radius)
{
    owner_ = owner;
    category_ = owner->GetCategory();
    radius_ = radius;

    Offset_ = { 0.0f, 0.0f, 0.0f };

    position_ = owner_->GetWorldPosition();

}

void Collider::Update()
{
    if(owner_){
    
        position_ = owner_->GetWorldPosition()+Offset_;
    }

}

void Collider::Draw()
{
#ifdef USE_LINE

    Sphere sphere;
    sphere.center = GetWorldPosition();
    sphere.radius = radius_;
    PrimitiveDrawer::GetInstance()->DrawSphere(sphere, { 1.0f, 0.0f, 0.0f, 1.0f });




#endif // USE_LINE


}



Vector3 Collider::GetWorldPosition() const
{
 if (owner_) {
        return owner_->GetWorldPosition()+Offset_;
    }
    return { 0.0f, 0.0f, 0.0f };
}

void Collider::OnCollision(Collider* other)
{

    if (owner_ && other && other->GetOwner()) {
        owner_->OnCollision(other->GetOwner());
    }
}
