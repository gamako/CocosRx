//
//  CCRxTouchEvent.h
//  cocos-rx
//
//  Created by gamako on 2014/08/28.
//
//

#ifndef __cocos_rx__CCRxTouchEvent__
#define __cocos_rx__CCRxTouchEvent__

#include "cocos2d.h"
#include "rx.hpp"

namespace CCRx {

    typedef rxcpp::observable<cocos2d::Touch*> TouchEventObservable;

    rxcpp::observable<std::tuple<cocos2d::Touch*, TouchEventObservable>>  touchEventObservable(
                                                                                               rxcpp::composite_subscription cs,
                                                                                               cocos2d::Node* targetNode,
                                                                                               std::function<bool(cocos2d::Touch*)> isBegan = nullptr,
                                                                                               bool isSwallow = false);

    inline rxcpp::observable<std::tuple<cocos2d::Touch*, TouchEventObservable>>  touchEventObservable(cocos2d::Node* targetNode,
                                                                                                      std::function<bool(cocos2d::Touch*)> isBegan = nullptr,
                                                                                                      bool isSwallow = false) {
        return touchEventObservable(rxcpp::composite_subscription(), targetNode, isBegan, isSwallow);
    }
}

#endif /* defined(__cocos_rx__CCRxTouchEvent__) */
