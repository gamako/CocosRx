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

    namespace touchObservable_detail {
        struct touchObservable_internal {
            rxsub::subject<std::tuple<Touch*, TouchEventObservable>> subject;
            RefPtr<Node> target;
            
            touchObservable_internal(Node* target) : subject(), target(target) {}
            touchObservable_internal(const touchObservable_internal& rhs) : subject(rhs.subject), target(rhs.target) {}
            touchObservable_internal(const touchObservable_internal&& rhs) : subject(std::move(rhs.subject)), target(std::move(rhs.target)) {}
            touchObservable_internal(rxsub::subject<std::tuple<Touch*, TouchEventObservable>> subject, Node* target) : subject(std::move(subject)), target(target) {}
            
            ~touchObservable_internal() {
            }
            
        private:
            touchObservable_internal() {}
        };
    }
    
    rx::observable<std::tuple<Touch*, TouchEventObservable>> touchEventObservable(rxcpp::composite_subscription cs, Node* targetNode, std::function<bool(Touch*)> isBegan /* = nullptr */, bool isSwallow /* = shallow */) {
        typedef rx::resource<touchObservable_detail::touchObservable_internal> resource_type;
        return rx::observable<>::scope(
                                       [targetNode, cs] () {
                                           return resource_type(touchObservable_detail::touchObservable_internal(targetNode), cs);
                                       },
                                       [isSwallow, isBegan] (resource_type r) {
                                           
                                           auto subscriber = r.get().subject.get_subscriber();
                                           
                                           auto listener = RefPtr<EventListenerTouchOneByOne>{EventListenerTouchOneByOne::create()};
                                           
                                           if (isSwallow) {
                                               listener->setSwallowTouches(isSwallow);
                                           }

                                           r.get_subscription().add([listener]() {
                                               Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
                                           });
                                           
                                           auto cs = r.get_subscription();
                                           auto finalizer = Util::shared_finallizer([cs]() {
                                               cs.unsubscribe();
                                           });

                                           listener->onTouchBegan = [isBegan, listener, subscriber, finalizer](Touch* t, Event*) -> bool {
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
                                           
                                           
                                           Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, r.get().target);
                                           
                                           return r.get().subject.get_observable();
                                       }).as_dynamic();

    }
    
    
}


