#ifndef __ALIEN_H__
#define __ALIEN_H__

#include "GameObject.h"

class Alien : public GameObject
{
public:
	Alien(void);
	~Alien(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
	void Shoot();

private:
	shared_ptr<Shape> mBulletShape;
};

#endif