#include <stdlib.h>
#include "GameUtil.h"
#include "Alien2.h"
#include "BoundingShape.h"

Alien2::Alien2(void) : GameObject("Alien2")
{
	mAngle = rand() % 360;
	mRotation = rand() % 90;
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	mVelocity.x = 10.0 * cos(DEG2RAD*mAngle);
	mVelocity.y = 10.0 * sin(DEG2RAD*mAngle);
	mVelocity.z = 0.0;
}

Alien2::~Alien2(void)
{
}

bool Alien2::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() == GameObjectType("Alien") || o->GetType() == GameObjectType("Alien2") || o->GetType() == GameObjectType("Asteroid") || o->GetType() == GameObjectType("Spaceship")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Alien2::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}
