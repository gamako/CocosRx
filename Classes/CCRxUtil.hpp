//
//  CCRxUtil.hpp
//  cocos-rx
//
//  Created by gamako on 2014/09/27.
//
//

#ifndef cocos_rx_CCRxUtil_hpp
#define cocos_rx_CCRxUtil_hpp

namespace CCRx {
    namespace Util {
        
        class shared_finallizer {
            struct dummy {
            };
            std::shared_ptr<dummy> b;
            std::function<void()> finalizer;
        public:
            shared_finallizer(std::function<void()> finalizer) : finalizer(finalizer) , b(std::make_shared<dummy>()) {}
            shared_finallizer(const shared_finallizer& rhs) : finalizer(rhs.finalizer), b(rhs.b) {}
            shared_finallizer(const shared_finallizer&& rhs) : finalizer(std::move(rhs.finalizer)), b(std::move(rhs.b)) {}
            shared_finallizer& operator = (const shared_finallizer& rhs) {
                finalizer = rhs.finalizer;
                b = rhs.b;
                return *this;
            }
            ~shared_finallizer() {
                if (b.unique()) {
                    finalizer();
                }
            }
        };
    }
}

#endif
