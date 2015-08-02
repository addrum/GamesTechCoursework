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
};

#endif
