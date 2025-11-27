#pragma once

#include <string>
namespace RealmEngine
{
    class Widget
    {
    public:
        explicit Widget(std::string name) : m_name(name), m_open(true) {}
        virtual ~Widget() = default;

        Widget(const Widget&)            = delete;
        Widget& operator=(const Widget&) = delete;
        Widget(Widget&&)                 = default;
        Widget& operator=(Widget&&)      = default;

        virtual void render() = 0;

        std::string getName() const { return m_name; }
        bool        isOpen() const { return m_open; }
        void        setOpen(bool open) { m_open = open; }

    protected:
        std::string m_name;
        bool        m_open;
    };

} // namespace RealmEngine
