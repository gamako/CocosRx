#ifndef __SHOOTING_SCENE_H__
#define __SHOOTING_SCENE_H__

#include "cocos2d.h"

class ShootingScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(ShootingScene);
};

#endif // __SHOOTING_SCENE_H__
