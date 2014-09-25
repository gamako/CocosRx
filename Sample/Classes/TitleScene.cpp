//
//  TitleScene.cpp
//  cocos-rx
//
//

#include "TitleScene.h"

#include "CCRxTouchEvent.h"
#include "CCRxScheduler.h"
#include "ShootingScene.h"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

Scene* TitleLayer::createScene()
{
    auto scene = Scene::create();
    auto layer = TitleLayer::create();
    scene->addChild(layer);
    return scene;
}

bool TitleLayer::init() {
    if (!Layer::init()) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto label = RefPtr<Label>(Label::createWithSystemFont("TOUCH TO START", "Arial", 24));
    this->addChild(label, 1);
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height/4));

    CCRx::interval(this, 1)
    .scan(false, [](bool b, float a) {
        return !b;
    })
    .as_dynamic()
    .subscribe([=](bool b) {
        label->setVisible(b);
    });
    
    CCRx::touchEventObservable(this, nullptr, false)
    .flat_map(rxu::apply_to([this](Touch *t, CCRx::TouchEventObservable o) {
        return o
        .last()
        .as_dynamic();
        
    }), [](std::tuple<Touch*, CCRx::TouchEventObservable>, Touch *t) {
        return t;
    })
    .subscribe([=](Touch *t) {
        auto scene = ShootingScene::createScene();
        Director::getInstance()->pushScene(scene);
    });
    
    
    return true;
}
