//
//  CCRxScheduler.cpp
//  cocos-rx
//
//  Created by gamako on 2014/08/30.
//
//

#include "CCRxScheduler.h"
#include <dispatch/dispatch.h>

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

namespace CCRx {
    
    namespace schedulers {
        
        namespace detail {
            
            bool is_current_main_thread() {
#ifdef _WIN64
#error
#elif _WIN32
#error
#elif __APPLE__
                return (dispatch_get_main_queue() == dispatch_get_current_queue());
#elif __linux || __unix || __posix
                return (getpid() == gettid());
#endif
            }
            
            static long long schedule_name_counter = 0;
            
            struct frame_update_scheduler : public rxcpp::schedulers::scheduler_interface
            {
            private:
                typedef frame_update_scheduler this_type;
                frame_update_scheduler(const this_type&);
                
                typedef rxcpp::schedulers::scheduler_base::clock_type clock;
//                typedef rxcpp::schedulers::detail::time_schedulable<clock::time_point> item_type;
//                typedef rxcpp::schedulers::detail::schedulable_queue<item_type::time_point_type> queue_item_time;
                
                struct frame_update_worker : public rxcpp::schedulers::worker_interface
                {
                private:
                    typedef frame_update_worker this_type;
                    
                    RefPtr<Node> targetNode;

                    frame_update_worker(const this_type&);
                public:
                    frame_update_worker(Node* targetNode) : targetNode(targetNode)
                    {
                    }
                    virtual ~frame_update_worker()
                    {
                    }
                    
                    void initialize() {
                    }
                    
                    virtual clock_type::time_point now() const {
                        return clock_type::now();
                    }
                    
                    virtual void schedule(const rxcpp::schedulers::schedulable& scbl) const {
                        schedule(now(), scbl);
                    }
                    
                    virtual void schedule(clock_type::time_point when, const rxcpp::schedulers::schedulable& scbl) const {
                        if (!scbl.is_subscribed()) {
                            return;
                        }
                        auto now = clock_type::now();
                        if (is_current_main_thread() && now >= when) {
                            rxcpp::schedulers::recursion r(true);
                            scbl(r.get_recurse());
                        } else {
                            {
                                auto name = StringUtils::format("CCRx-update %lld", schedule_name_counter);
                                schedule_name_counter++;
                                float delay = std::chrono::duration<float>(when - now).count();
                                Director::getInstance()->getScheduler()->schedule(
                                                                                  [scbl](float delta) {
                                                                                      rxcpp::schedulers::recursion r(true);
                                                                                      scbl(r.get_recurse());
                                                                                  },
                                                                                  targetNode.get(),
                                                                                  0.0, 0, delay, false, name);
                            }
                            
                        }
                    }
                };
                
                RefPtr<Node> targetNode;
                std::shared_ptr<frame_update_worker> wi;
                
            public:
                explicit frame_update_scheduler(Node* node)
                : wi(new frame_update_worker(node)) , targetNode(node)
                {
                }
                virtual ~frame_update_scheduler()
                {
                }
                
                virtual clock_type::time_point now() const {
                    return clock_type::now();
                }
                
                virtual rxcpp::schedulers::worker create_worker(rxcpp::composite_subscription cs) const {
                    return rxcpp::schedulers::worker(std::move(cs), wi);
                }
            };
            
        }
        rxcpp::schedulers::scheduler make_frame_update_scheduler(Node *node) {
            static auto ct = rxcpp::schedulers::make_scheduler<detail::frame_update_scheduler>(node);
            return ct;
        }
    }
    
    
    static int counter = 0;
    
    namespace interval_detail {
        struct interval_internal {
            int counter;
            rxsub::subject<float> subject;
            RefPtr<Node> target;
            
            interval_internal(int counter, Node* target) : counter(counter), subject(), target(target) {}
            interval_internal(const interval_internal& rhs) : counter(rhs.counter), subject(rhs.subject), target(rhs.target) {}
            interval_internal(const interval_internal&& rhs) : counter(rhs.counter), subject(rhs.subject), target(rhs.target) {}
            interval_internal(int counter, rxsub::subject<float> subject, Node* target) : counter(counter), subject(std::move(subject)), target(target) {}
            
            ~interval_internal() {
                Director::getInstance()->getScheduler()->unschedule(event_key(), target);
            }
            
            const std::string event_key() const {
                return StringUtils::format("CCRx_interval_%d", counter);
            }
        private:
            interval_internal() {}
        };
    }
    
    rx::observable<float> interval(Node* targetNode, float interval) {
        typedef rx::resource<interval_detail::interval_internal> resource_type;
        return rx::observable<>::scope(
                                       [targetNode] () {
                                           return resource_type(interval_detail::interval_internal(counter++, targetNode));
                                       },
                                       [interval] (resource_type r) {
                                           
                                           auto subscriber = r.get().subject.get_subscriber();

                                           Director::getInstance()->getScheduler()->schedule(
                                                                                             [subscriber, r](float delta) {
                                                                                                 subscriber.on_next(delta);
                                                                                             },
                                                                                             r.get().target.get(),
                                                                                             interval, kRepeatForever, 0.0, false, r.get().event_key());

                                           
                                           return r.get().subject.get_observable();
                                       }).as_dynamic();
    }
}


