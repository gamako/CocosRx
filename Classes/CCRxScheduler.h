//
//  CCRxScheduler.h
//  cocos-rx
//
//  Created by gamako on 2014/08/30.
//
//

#ifndef __cocos_rx__CCRxScheduler__
#define __cocos_rx__CCRxScheduler__

#include "cocos2d.h"
#include "rx.hpp"

namespace CCRx {
    namespace schedulers {
        rxcpp::schedulers::scheduler make_frame_update_scheduler(cocos2d::Node *node);
    }

    rxcpp::observable<float> interval(cocos2d::Node* targetNode, float interval);
}

#endif /* defined(__cocos_rx__CCRxScheduler__) */
