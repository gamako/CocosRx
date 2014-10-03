#include "ShootingScene.h"
#include "rxcpp/rx.hpp"
#include <memory>
#include <random>

#include "CCRxTouchEvent.h"
#include "CCRxScheduler.h"

#include "TitleScene.h"

namespace rx=rxcpp;
namespace rxu=rxcpp::util;
namespace rxsc=rxcpp::schedulers;
namespace rxsub=rxcpp::subjects;

USING_NS_CC;

template <typename PrimitiveType>
class ObservableValue {
    struct InnerValue {
        PrimitiveType value;
        rxsub::subject<PrimitiveType> subject;

        explicit InnerValue(PrimitiveType initialValue) : value(initialValue), subject() {}
    };
    
public:
    std::shared_ptr<InnerValue> innerValue;
    explicit ObservableValue(PrimitiveType initialValue) : innerValue(std::make_shared<InnerValue>(initialValue)) {}
    ObservableValue(const ObservableValue& changeObservable) : innerValue(changeObservable.innerValue) {}
    ObservableValue(ObservableValue&& changeObservable) : innerValue(std::move(changeObservable.innerValue)) {}

    PrimitiveType getValue() const { return innerValue->value; }
    
    void setValue(PrimitiveType value) const {
        const PrimitiveType& oldValue = innerValue->value;
        if (oldValue != value) {
            innerValue->value = value;
            innerValue->subject.get_subscriber().on_next(value);
        }
    }
    
    rx::observable<PrimitiveType> get_observable() {
        return innerValue->subject.get_observable();
    }
};

Scene* ShootingScene::createScene()
{
    auto scene = Scene::create();
    auto layer = ShootingScene::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool ShootingScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    enum TagType : int {
        Player = 0,
        Shot,
        Enemy,
    };
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    ObservableValue<int> score(0);
    
    // Collision Detection Observable
    typedef std::tuple<Node*, Node*> CollisionArg;
    auto collisionObservable = rx::observable<>::scope([]() {
        return rx::resource<rxsub::subject<CollisionArg>>(rxsub::subject<CollisionArg>());
    }, [=](rx::resource<rxsub::subject<CollisionArg>> resource) {

        auto subscriber = resource.get().get_subscriber();
        
        CCRx::interval(this, 0)
        .as_dynamic()
        .subscribe([=](float) {
            Vector<Node*> children(this->getChildren());
            
            rx::observable<>::iterate(children).as_dynamic()
            .subscribe([=](Node* child1) {
                
                bool isTrail = false;
                rx::observable<>::iterate(children)
                .filter([=, &isTrail](Node *n) { if (n == child1) { isTrail = true; } return !isTrail; })
                .as_dynamic()
                .subscribe([=](Node *child2) {
                    if (child1->getBoundingBox().intersectsRect(child2->getBoundingBox())) {
                        subscriber.on_next(std::make_tuple(child1, child2));
                    }
                });
            });
        });

        return resource.get().get_observable();
    }).publish().as_dynamic();
    auto collision_subscription = collisionObservable.connect();
 
    // create sprites
    auto player = Sprite::create("player.png");
    player->setTag(TagType::Player);
    addChild(player);
    player->setPosition(Vec2{visibleSize.width/2, 200});
    
    // create touch event
    auto touchObservableObservable = CCRx::touchEventObservable(this, nullptr, false);

    // move player sprite according to touch and move
    auto shared_player = RefPtr<Sprite>(player);
    touchObservableObservable.subscribe(rxu::apply_to([=](Touch *t, CCRx::TouchEventObservable o) {
        
        auto touchPointOrigin = this->getParent()->convertToNodeSpace(t->getLocation());
        auto playerOrigin = shared_player->getPosition();

        o.subscribe(
                    [=](Touch* t){

                        auto touchPoint = this->getParent()->convertToNodeSpace(t->getLocation());
                        auto newPosition = playerOrigin + (touchPoint - touchPointOrigin);
                        shared_player->setPosition(std::max(std::min(newPosition.x, visibleSize.width), 0.0f), playerOrigin.y);

                    },
                    [](std::exception_ptr){ CCLOG("error"); },
                    [](){ CCLOG("completed"); });
    }));

    
    collisionObservable
    .filter(rxu::apply_to([=](Node* a, Node* b) {
        return (a == shared_player.get() || b == shared_player.get());
    }))
    .map(rxu::apply_to([=](Node* a, Node* b) {
        if (a == shared_player.get()) {
            return b;
        } else {
            return a;
        }
    }))
    .subscribe([=](Node* a) {
        if (a->getTag() == TagType::Enemy) {
            shared_player->removeFromParent();
            
            auto label = LabelTTF::create("Game Over", "Arial", 48);
            this->addChild(label, 1);
            label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                    origin.y + visibleSize.height/2));

            
            touchObservableObservable.subscribe(rxu::apply_to([=](Touch *t, CCRx::TouchEventObservable o) {
                auto scene = TitleLayer::createScene();
                Director::getInstance()->replaceScene(scene);
                collision_subscription.unsubscribe();
            }));

        }
    });
    
    // make single-tap event
    touchObservableObservable
    .flat_map(rxu::apply_to([this, shared_player](Touch *t, CCRx::TouchEventObservable o) {
        const auto beginTime = std::chrono::system_clock::now();

        return o
        .last()
        .map([](Touch *) { return std::chrono::system_clock::now(); })
        .filter([=](std::chrono::system_clock::time_point time) {
            return ((time - beginTime) < std::chrono::duration<float>(0.2));
        })
        .as_dynamic();
        
    }), [](std::tuple<Touch*, CCRx::TouchEventObservable>, std::chrono::system_clock::time_point d) {
        return d;
    })
    .subscribe([=](std::chrono::system_clock::time_point) {
        auto shot = RefPtr<Sprite>(Sprite::create("shot.png"));
        shot->setTag(TagType::Shot);
        addChild(shot.get());
        
        auto startPosition = shared_player->getPosition() + Vec2(0, 50);
        shot->setPosition(startPosition);
        const float speed = 1000;
        auto vector = Vec2{0, 1} * speed;

        auto shotCollisionDetectionSubscription = rx::composite_subscription();
        CCRx::interval(shot.get(), 0)
        .scan(0.0f, [](float sum, float d) { return sum + d; })
        .as_dynamic()
        .subscribe([=](float delta) {
            shot->setPosition(startPosition + vector * delta);
            
            if (shot->getPosition().y > visibleSize.height) {
                shotCollisionDetectionSubscription.unsubscribe();
                shot->removeFromParent();
            }
        });
        
        collisionObservable
        .filter(rxu::apply_to([=](Node* a, Node* b) {
            return (a == shot.get() || b == shot.get());
        }))
        .map(rxu::apply_to([=](Node* a, Node* b) {
            if (a == shot.get()) {
                return b;
            } else {
                return a;
            }
        }))
        .filter([](Node *a) { return a->getParent() != nullptr; })
        .subscribe(shotCollisionDetectionSubscription, [=](Node* a) {
            if (a->getTag() == TagType::Enemy) {
                shotCollisionDetectionSubscription.unsubscribe();
                a->removeFromParent();
                shot->removeFromParent();
                score.setValue(score.getValue() + 100);
            }
        });
    });
    
    auto rnd = std::make_shared<std::mt19937>();
    
    CCRx::interval(this, 3)
    .start_with(0)
    .as_dynamic()
    .subscribe([=](float) {
        std::uniform_real_distribution<float> xDist(0.0 ,visibleSize.width);
        const auto fromX = xDist(*rnd);
        const auto toX = xDist(*rnd);
        const auto startPosition = Vec2{fromX, visibleSize.height};
        const float speed = 100.0;
        const auto vector = (Vec2{toX, 0} - startPosition).getNormalized() * speed;

        auto enemy = RefPtr<Sprite>(Sprite::create("enemy.png"));
        enemy->setTag(TagType::Enemy);
        addChild(enemy.get());
        
        enemy->setPosition(startPosition);

        CCRx::interval(enemy.get(), 0)
        .scan(0.0f, [](float sum, float b) { return sum + b; })
        .as_dynamic()
        .subscribe([=](float delta) {
            enemy->setPosition(startPosition + vector * delta);
            
            if (enemy->getPosition().y < 0) {
                enemy->removeFromParent();
            }
        });
        
    });
    

    // Score
    auto scoreLabel = LabelTTF::create("0", "Arial", 24);
    this->addChild(scoreLabel, 1);

    score.get_observable()
    .start_with(0)
    .as_dynamic()
    .subscribe([=](int newScore) {
        scoreLabel->setString(cocos2d::StringUtils::format("Score: %4d", newScore));
        scoreLabel->setPosition(Vec2(origin.x + visibleSize.width/2,
                                     origin.y + visibleSize.height - scoreLabel->getContentSize().height));
        
    });

    return true;
}
