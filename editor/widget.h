#pragma once

#include <string>
namespace RealmEngine
{
    class Widget
    {
    public:
        explicit Widget(const std::string& name) : m_name(name), m_open(true) {}
        explicit Widget(std::string&& name) : m_name(std::move(name)), m_open(true) {}
        virtual ~Widget() noexcept = default;

        Widget(const Widget&)                = delete;
        Widget& operator=(const Widget&)     = delete;
        Widget(Widget&&) noexcept            = default;
        Widget& operator=(Widget&&) noexcept = default;

        virtual void render() = 0;

        std::string  getName() const { return m_name; }
        virtual bool isOpen() const { return m_open; }
        void         setOpen(const bool open) { m_open = open; }

    protected:
        std::string m_name;
        bool        m_open;
    };

} // namespace RealmEngine
