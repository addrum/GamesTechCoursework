#include <stdlib.h>
#include "GameUtil.h"
#include "Alien2.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "Bullet.h"

Alien2::Alien2(void) : GameObject("Alien2")
{
	mAngle = rand() % 360;
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

/** Shoot a bullet. */
void Alien2::Shoot(void)
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

void Alien2::Update(int t)
{
	shared_ptr<GameObject> player = GetPlayer();
	if (player != NULL)
	{
		if (RangeTest(player))
		{
			Shoot();
		}
		GLVector3f alien_heading(cos(DEG2RAD*mAngle), sin(DEG2RAD*mAngle), 0);
		alien_heading.normalize();
		this->SetAngle(player->GetAngle());
	}
	GameObject::Update(t);
}

bool Alien2::RangeTest(shared_ptr<GameObject> o)
{
	if (o->GetPosition() < GetPosition() * 2 && o->GetPosition() > GetPosition() / 2)
	{
		return true;
	}
	return false;
}

shared_ptr<GameObject> Alien2::GetPlayer()
{
	GameObjectList objects = mWorld->GetObjects();
	shared_ptr<GameObject> player;
	list<shared_ptr<GameObject>>::iterator iter;

	for (iter = objects.begin(); iter != objects.end(); ++iter)
	{
		if ((*iter)->GetType() == GameObjectType("Spaceship")) 
		{
			player = (*iter);
			return player;
		}
	}
	return NULL;
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
