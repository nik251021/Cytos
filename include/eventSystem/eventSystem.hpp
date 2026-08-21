#pragma once
#include <entt/entt.hpp>
#include <iostream>

struct CollisionEvent {
    entt::entity entityA;
    entt::entity entityB;
};

class EventSystem {
private:
    inline static entt::dispatcher s_dispatcher;

public:
    static entt::dispatcher& getDispatcher() {
        return s_dispatcher;
    }

    template<typename Event, typename... Args>
    static void trigger(Args&&... args) {
        s_dispatcher.trigger<Event>(std::forward<Args>(args)...);
    }
    
    template<typename Event>
    static void enqueue(const Event& event) {
        s_dispatcher.enqueue<Event>(event);
    }

    static void update() {
        s_dispatcher.update();
    }
};