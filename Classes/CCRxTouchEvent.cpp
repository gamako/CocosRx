//
//  CCRxTouchEvent.cpp
//  cocos-rx
//
//  Created by gamako on 2014/08/28.
//
//

#include "CCRxTouchEvent.h"
#include "CCRxUtil.hpp"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

namespace CCRx {

    rx::observable<std::tuple<Touch*, TouchEventObservable>> touchEventObservable(Node* targetNode, std::function<bool(Touch*)> isBegan /* = nullptr */, bool isSwallow /* = shallow */) {

        RefPtr<Node> sharedTarget(targetNode);
        return rx::observable<>::defer(
                                       [sharedTarget, isSwallow, isBegan] () {
                                           auto subject = std::make_shared<rxsub::subject<std::tuple<Touch*, TouchEventObservable>>>();
                                           auto subscriber = subject->get_subscriber();
                                           
                                           auto listener = RefPtr<EventListenerTouchOneByOne>{EventListenerTouchOneByOne::create()};
                                           
                                           if (isSwallow) {
                                               listener->setSwallowTouches(isSwallow);
                                           }

                                           auto finalizer = Util::shared_finallizer([subject, subscriber]() {
                                               subscriber.on_completed();
                                           });

                                           listener->onTouchBegan = [isBegan, listener, subscriber, finalizer, subject](Touch* t, Event*) -> bool {
                                               if (isBegan) {
                                                   if (!isBegan(t)) {
                                                       return false;
                                                   }
                                               }
                                               rxsub::subject<Touch*> subj2{};
                                               auto subscriber2 = subj2.get_subscriber();
                                               listener->onTouchMoved = [=](Touch* t, Event*) {
                                                   subscriber2.on_next(t);
                                               };
                                               listener->onTouchEnded = [=](Touch* t, Event*) {
                                                   subscriber2.on_next(t);
                                                   subscriber2.on_completed();
                                               };
                                               listener->onTouchCancelled = [=](Touch* t, Event*) {
                                                   subscriber2.on_next(t);
                                                   subscriber2.on_completed();
                                               };
                                               
                                               auto ob = subj2.get_observable();
                                               subscriber.on_next(std::make_tuple(t, ob));
                                               return true;
                                           };
                                           
                                           
                                           Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, sharedTarget.get());
                                           
                                           return subject->get_observable()
                                           .finally([=]() {
                                               subscriber.unsubscribe();
                                               Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
                                           }).as_dynamic();
                                       }).as_dynamic();

    }
    
    
}


