#ifndef __ALIEN2_H__
#define __ALIEN2_H__

#include "GameObject.h"

class Alien2 : public GameObject
{
public:
	Alien2(void);
	~Alien2(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
	void RotateToPlayerDirection(shared_ptr<GameObject> o);
	void Shoot();
	void Update(int t);
	bool RangeTest(shared_ptr<GameObject> o);
	shared_ptr<GameObject> GetPlayer();

private:
	shared_ptr<Shape> mBulletShape;
};

#endif
