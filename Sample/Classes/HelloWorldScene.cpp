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
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

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

    {
        // touchlog
//        auto touchObOb = CCRx::touchEventObservable(this, nullptr, false);
//        
//        touchObOb.subscribe([](CCRx::TouchEventObservable o) {
//            CCLOG("touch");
//            o.subscribe([](Touch* t){ CCLOG("move"); },
//                        [](std::exception_ptr){ CCLOG("error"); },
//                        [](){ CCLOG("completed"); });
//        });

        // interval
//        CCRx::interval(this, 1.0)
//        .subscribe([](int i){ CCLOG("interval %d", i); },
//                   [](std::exception_ptr){ CCLOG("error"); },
//                   [](){ CCLOG("completed"); });

        
        {
            using namespace std::chrono;
            typedef steady_clock clock;
            
            int c = 0;
            //auto sc = rxsc::make_current_thread();
            //auto sc = rxsc::make_event_loop();
            //auto sc = rxsc::make_immediate();
            //auto sc = CCRx::schedulers::make_frame_update_scheduler(this);
            auto sc = rxsc::make_new_thread();
            auto so = rx::synchronize_in_one_worker(sc);
            auto start = sc.now() + seconds(0);
            auto period = seconds(1);
            rx::composite_subscription cs;
            rx::observable<>::interval(start, period, so)
//            .observe_on(rx::synchronize_in_one_worker(rxsc::make_current_thread()))
            .observe_on(rx::synchronize_in_one_worker(CCRx::schedulers::make_frame_update_scheduler(this)))
            .subscribe(
                       cs,
                       [=, &c](long counter){
                           auto nsDelta = duration_cast<milliseconds>(sc.now() - (start + (period * (counter - 1))));
                           c = counter - 1;
                           std::cout << "interval          : period " << counter << ", " << nsDelta.count() << "ms delta from target time" << std::endl;
                           if (counter == 5) {cs.unsubscribe();}
                       },
                       [](std::exception_ptr){abort();});

        }
        
        // ダブルタップ
//        auto interval = CCRx::schedule(this, 5.0, "aa");
//
//        auto touchObservable = disposableObserbable.get_observable()
//        .flat_map([](rx::observable<Touch*> o) { return o.last().as_dynamic(); },
//                  [](rx::observable<Touch*> o, Touch* t) { return t; })
//        .as_dynamic().publish();
//        touchObservable.connect();
//        
//        auto count = std::make_shared<int>(0);
//        touchObservable
//        .skip_until(interval.get_observable())
//        .subscribe([count](Touch* t){ CCLOG("move %d", *(count.get())); ++(*(count.get())); },
//                        [](std::exception_ptr){ CCLOG("error"); },
//                        [](){ CCLOG("completed"); });
        

        
    };

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
