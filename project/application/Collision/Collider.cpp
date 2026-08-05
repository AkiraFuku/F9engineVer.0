#include "Collider.h"

#include"GameObject.h"

void Collider::initialize(GameObject* owner, float radius)
{
    owner_ = owner;
    category_ = owner->GetCategory();
    radius_ = radius;
}



Vector3 Collider::GetWorldPosition() const
{
 if (owner_) {
        return owner_->GetWorldPosition();
    }
    return { 0.0f, 0.0f, 0.0f };
}

void Collider::OnCollision(Collider* other)
{

    if (owner_ && other && other->GetOwner()) {
        owner_->OnCollision(other->GetOwner());
    }
}
