#pragma once
#include <iostream>
#include <utility>
#include <string>


#include "Vector3.h"

std::pair<Vector3,Vector3> ComputeCollisionVelocities(float mass1,const Vector3& velocity1,float mass2,const Vector3& velocity2,float coefficientOfRestitution,const Vector3& normal);