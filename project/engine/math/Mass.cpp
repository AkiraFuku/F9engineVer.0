#include "Mass.h"
#include "DrawFunction.h"
#include "MathFunction.h"
std::pair<Vector3, Vector3> ComputeCollisionVelocities(
	float mass1, const Vector3& velocity1, float mass2, const Vector3& velocity2,float coefficientOfRestitution,const Vector3& normal)
{


	Vector3 project1= Project(velocity1,normal);
	Vector3 project2= Project(velocity2,normal);
	Vector3 sub1= velocity1-project1;
	Vector3 sub2= velocity2-project2;

	float commonDenom = mass1 + mass2;


	Vector3 velocityAfter1={
	
		{normal.x*((mass1 - coefficientOfRestitution * mass2) * project1.x + (1.0f + coefficientOfRestitution) * mass2 * project2.x) / commonDenom},
		{normal.y*((mass1 - coefficientOfRestitution * mass2) * project1.y + (1.0f + coefficientOfRestitution) * mass2 * project2.y) / commonDenom},
		{normal.z*((mass1 - coefficientOfRestitution * mass2) * project1.z + (1.0f + coefficientOfRestitution) * mass2 * project2.z) / commonDenom}
	};


	Vector3 velocityAfter2={
		{normal.x* ((1.0f + coefficientOfRestitution) * mass1 * project1.x + (mass2 - coefficientOfRestitution * mass1) * project2.x) / commonDenom},
		{normal.y* ((1.0f + coefficientOfRestitution) * mass1 * project1.y + (mass2 - coefficientOfRestitution * mass1) * project2.y) / commonDenom},
		{normal.z* ((1.0f + coefficientOfRestitution) * mass1 * project1.z + (mass2 - coefficientOfRestitution * mass1) * project2.z) / commonDenom}
	};


	return std::make_pair(velocityAfter1+sub1,velocityAfter2+sub1);
}
