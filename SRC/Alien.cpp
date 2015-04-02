#include <stdlib.h>
#include "GameUtil.h"
#include "Alien.h"
#include "Bullet.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"

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

/** Shoot a bullet. */
void Alien::Shoot(void)
{
	// Check the world exists
	if (!mWorld) return;
	// Construct a unit length vector in the direction the spaceship is headed
	GLVector3f alien_heading(cos(DEG2RAD*mAngle), sin(DEG2RAD*mAngle), 0);
	alien_heading.normalize();
	// Calculate the point at the node of the spaceship from position and heading
	GLVector3f bullet_position = mPosition + (alien_heading * 4);
	// Calculate how fast the bullet should travel
	float bullet_speed = 30;
	// Construct a vector for the bullet's velocity
	GLVector3f bullet_velocity = mVelocity + alien_heading * bullet_speed;
	// Construct a new bullet
	shared_ptr<GameObject> bullet
		(new Bullet(bullet_position, bullet_velocity, mAcceleration, mAngle, 0, 2000));
	bullet->SetBoundingShape(make_shared<BoundingSphere>(bullet->GetThisPtr(), 2.0f));
	bullet->SetShape(mBulletShape);
	// Add the new bullet to the game world
	mWorld->AddObject(bullet);

}

bool Alien::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() == GameObjectType("Alien") || o->GetType() == GameObjectType("Asteroid") || o->GetType() == GameObjectType("Spaceship")) return false;
	if (o->GetType() == GameObjectType("Bullet"))
	{
		GLVector3f bullet_direction(cos(DEG2RAD * o->GetAngle()), sin(DEG2RAD* o->GetAngle()), 0);
		bullet_direction.normalize();
		SetPosition(mPosition + (bullet_direction / 10));
	}
	else
	{
		SetPosition(mPosition);
	}
	if (o->GetType() == GameObjectType("Spaceship"))
	{
		Shoot();
	}
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Alien::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}
