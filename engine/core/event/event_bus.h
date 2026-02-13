#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace RealmEngine
{
    /// <summary>
    /// Type-erased publish/subscribe event bus.
    /// Subscribers register with a typed handler; the bus dispatches by event type at runtime.
    /// Thread-safe for subscribe/unsubscribe; publish must happen on the main thread.
    /// </summary>
    class EventBus
    {
    public:
        using HandlerId = uint64_t;

        EventBus()           = default;
        ~EventBus() noexcept = default;

        EventBus(const EventBus&)            = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&)                 = delete;
        EventBus& operator=(EventBus&&)      = delete;

        // Subscribe a callable to events of type E. Returns a handle for unsubscribe.
        template<typename E>
        HandlerId subscribe(std::function<void(const E&)> handler)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            HandlerId                   id       = m_next_id++;
            auto                        type_key = std::type_index(typeid(E));

            m_handlers[type_key].push_back(
                {id, [handler = std::move(handler)](const void* raw) { handler(*static_cast<const E*>(raw)); }});
            return id;
        }

        // Remove a previously registered handler by its id
        void unsubscribe(HandlerId id)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& [type, entries] : m_handlers)
            {
                auto it =
                    std::remove_if(entries.begin(), entries.end(), [id](const HandlerEntry& e) { return e.id == id; });
                if (it != entries.end())
                {
                    entries.erase(it, entries.end());
                    return;
                }
            }
        }

        // Publish an event to all subscribers of type E
        template<typename E>
        void publish(const E& event) const
        {
            auto type_key = std::type_index(typeid(E));
            auto it       = m_handlers.find(type_key);
            if (it == m_handlers.end())
                return;

            for (const auto& entry : it->second)
                entry.func(&event);
        }

    private:
        using ErasedHandler = std::function<void(const void*)>;

        struct HandlerEntry
        {
            HandlerId     id;
            ErasedHandler func;
        };

        std::unordered_map<std::type_index, std::vector<HandlerEntry>> m_handlers;
        HandlerId                                                      m_next_id {0};
        mutable std::mutex                                             m_mutex;
    };

} // namespace RealmEngine
