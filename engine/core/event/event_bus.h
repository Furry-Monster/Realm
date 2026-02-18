#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace RealmEngine
{
    /**
     * @brief
     * Type-erased publish/subscribe event bus.
     * Thread-safe for subscribe/unsubscribe; publish must happen on the main thread.
     */
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

        /**
         * @brief
         * Subscribe a callable to events of type E.
         * Returns a handle for unsubscribe.
         *
         * @param handler callable to events of type E
         * @return template<typename E> handle for unsubscribe
         */
        template<typename E>
        [[nodiscard]] HandlerId subscribe(std::function<void(const E&)> handler)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            HandlerId                   id       = m_next_id++;
            const auto                  type_key = std::type_index(typeid(E));

            m_handlers[type_key].push_back(
                {id, [handler = std::move(handler)](const void* raw) { handler(*static_cast<const E*>(raw)); }});
            return id;
        }

        /**
         * @brief
         * Remove a previously registered handler by its id
         *
         * @param id
         */
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

        /**
         * @brief
         * Publish an event to all subscribers of type E
         *
         * @tparam E
         * @param event
         */
        template<typename E>
        void publish(const E& event) const
        {
            std::vector<ErasedHandler> snapshot;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                const auto                  type_key = std::type_index(typeid(E));
                const auto                  it       = m_handlers.find(type_key);
                if (it == m_handlers.end())
                    return;
                snapshot.reserve(it->second.size());
                for (const auto& entry : it->second)
                    snapshot.push_back(entry.func);
            }
            for (const auto& func : snapshot)
                func(&event);
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
