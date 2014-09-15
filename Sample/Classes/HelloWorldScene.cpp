#include "HelloWorldScene.h"
#include "rxcpp/rx.hpp"
#include <memory>

#include "CCRxTouchEvent.h"
#include "CCRxScheduler.h"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // create sprites
    auto player = Sprite::create("player.png");
    addChild(player);
    player->setPosition(Vec2{visibleSize.width/2, 200});
    
    // create touch event
    auto touchObservableObservable = CCRx::touchEventObservable(this, nullptr, false);

    // move player sprite according to touch and move
    auto shared_player = RefPtr<Sprite>(player);
    touchObservableObservable.subscribe(rxu::apply_to([this, shared_player](Touch *t, CCRx::TouchEventObservable o) {
        
        auto touchPointOrigin = this->getParent()->convertToNodeSpace(t->getLocation());
        auto playerOrigin = shared_player->getPosition();

        o.subscribe(
                    [this, shared_player, touchPointOrigin, playerOrigin](Touch* t){

                        auto touchPoint = this->getParent()->convertToNodeSpace(t->getLocation());
                        auto newPosition = playerOrigin + (touchPoint - touchPointOrigin);
                        shared_player->setPosition(newPosition.x, playerOrigin.y);

                    },
                    [](std::exception_ptr){ CCLOG("error"); },
                    [](){ CCLOG("completed"); });
    }));

    // make single-tap event
    touchObservableObservable
    .flat_map(rxu::apply_to([this, shared_player](Touch *t, CCRx::TouchEventObservable o) {
        typedef std::chrono::duration<float> float_seconds;
        auto beginTime = std::chrono::system_clock::now();
        return o
        .last()
        .map([=](Touch *) { return std::chrono::duration_cast<float_seconds>(std::chrono::system_clock::now() - beginTime).count(); })
        .filter([](float d) { return (d < 0.2); })
        .as_dynamic();
        
    }), [](std::tuple<Touch*, CCRx::TouchEventObservable>, float d) {
        return d;
    })
    .subscribe([=](float d) {
        auto shot = RefPtr<Sprite>(Sprite::create("shot.png"));
        addChild(shot.get());
        
        auto startPosition = shared_player->getPosition() + Vec2(0, 50);
        shot->setPosition(startPosition);
        auto speed = Vec2{0, 1000};
        auto startTime = std::chrono::system_clock::now();
        
        CCRx::interval(shot.get(), 0)
        .as_dynamic()
        .subscribe([=](float d) {
            typedef std::chrono::duration<float> float_seconds;
            auto now = std::chrono::system_clock::now();
            auto spendTime = std::chrono::duration_cast<float_seconds>(now - startTime).count();
            
            shot->setPosition(startPosition + speed * spendTime);
            
            if (shot->getPosition().y > visibleSize.height) {
                shot->removeFromParent();
            }
        });
    });
    
    
    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
