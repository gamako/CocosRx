//
//  TitleScene.h
//  cocos-rx
//

#ifndef __cocos_rx__TitleScene__
#define __cocos_rx__TitleScene__

#include "cocos2d.h"

class TitleLayer : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    
    CREATE_FUNC(TitleLayer);
};


#endif /* defined(__cocos_rx__TitleScene__) */
