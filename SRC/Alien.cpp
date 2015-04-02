#include <stdlib.h>
#include "GameUtil.h"
#include "Alien.h"
#include "BoundingShape.h"

Alien::Alien(void) : GameObject("Alien")
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

Alien::~Alien(void)
{
}

bool Alien::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() == GameObjectType("Alien") || o->GetType() == GameObjectType("Asteroid") || o->GetType() == GameObjectType("Spaceship")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Alien::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}
