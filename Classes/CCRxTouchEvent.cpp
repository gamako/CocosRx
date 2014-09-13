//
//  CCRxTouchEvent.cpp
//  cocos-rx
//
//  Created by gamako on 2014/08/28.
//
//

#include "CCRxTouchEvent.h"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

namespace CCRx {

    namespace touchObservable_detail {
        struct touchObservable_internal {
            rxsub::subject<TouchEventObservable> subject;
            RefPtr<Node> target;
            
            touchObservable_internal(Node* target) : subject(), target(target) {}
            touchObservable_internal(const touchObservable_internal& rhs) : subject(rhs.subject), target(rhs.target) {}
            touchObservable_internal(const touchObservable_internal&& rhs) : subject(rhs.subject), target(rhs.target) {}
            touchObservable_internal(rxsub::subject<TouchEventObservable> subject, Node* target) : subject(std::move(subject)), target(target) {}
            
            ~touchObservable_internal() {
            }
            
        private:
            touchObservable_internal() {}
        };
    }
    
    rx::observable<TouchEventObservable> touchEventObservable(Node* targetNode, std::function<bool(Touch*)> isBegan /* = nullptr */, bool isSwallow /* = shallow */) {
        typedef rx::resource<touchObservable_detail::touchObservable_internal> resource_type;
        return rx::observable<>::scope(
                                       [targetNode] () {
                                           return resource_type(touchObservable_detail::touchObservable_internal(targetNode));
                                       },
                                       [isSwallow, isBegan] (resource_type r) {
                                           
                                           auto subscriber = r.get().subject.get_subscriber();
                                           
                                           auto listener = RefPtr<EventListenerTouchOneByOne>{EventListenerTouchOneByOne::create()};
                                           
                                           if (isSwallow) {
                                               listener->setSwallowTouches(isSwallow);
                                           }
                                           
                                           listener->onTouchBegan = [isBegan, listener, subscriber](Touch* t, Event*) -> bool {
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
                                               subscriber.on_next(ob);
                                               return true;
                                           };
                                           
                                           
                                           Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, r.get().target);
                                           
                                           return r.get().subject.get_observable();
                                       }).as_dynamic();

    
//    rx::observable<TouchEventObservableObservable> touchEventObservable(Node* targetNode, std::function<bool(Touch*)> isBegan /* = nullptr */, bool isSwallow /* = shallow */) {
//        typedef rx::resource<touchObservable_detail::touchObservable_internal> resource_type;
//        return rx::observable<>::scope(
//                                       [targetNode] () {
//                                           return resource_type(touchObservable_detail::touchObservable_internal(targetNode));
//                                       },
//                                       [isSwallow, isBegan] (resource_type r) {
//                                           
//                                           auto subscriber = r.get().subject.get_subscriber();
//
//                                           auto listener = RefPtr<EventListenerTouchOneByOne>{EventListenerTouchOneByOne::create()};
//                                           
//                                           if (isSwallow) {
//                                               listener->setSwallowTouches(isSwallow);
//                                           }
//                                           
//                                           listener->onTouchBegan = [isBegan, listener, subscriber](Touch* t, Event*) -> bool {
//                                               if (isBegan) {
//                                                   if (!isBegan(t)) {
//                                                       return false;
//                                                   }
//                                               }
//                                               rxsub::subject<Touch*> subj2{};
//                                               auto subscriber2 = subj2.get_subscriber();
//                                               listener->onTouchMoved = [=](Touch* t, Event*) {
//                                                   subscriber2.on_next(t);
//                                               };
//                                               listener->onTouchEnded = [=](Touch* t, Event*) {
//                                                   subscriber2.on_next(t);
//                                                   subscriber2.on_completed();
//                                               };
//                                               listener->onTouchCancelled = [=](Touch* t, Event*) {
//                                               };
//                                               
//                                               auto ob = subj2.get_observable();
//                                               subscriber.on_next(ob);
//                                               return true;
//                                           };
//                                           
//                                           
//                                           Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, r.get().target);
//                                           
//                                           return r.get().subject.get_observable();
//                                       }).as_dynamic();

    }
    
    
}


