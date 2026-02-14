#pragma once

#include "widget.h"

namespace RealmEngine
{
    class ProfilerWidget : public Widget
    {
    public:
        ProfilerWidget();
        ~ProfilerWidget() override = default;

        ProfilerWidget(const ProfilerWidget&)            = delete;
        ProfilerWidget& operator=(const ProfilerWidget&) = delete;
        ProfilerWidget(ProfilerWidget&&)                 = delete;
        ProfilerWidget& operator=(ProfilerWidget&&)      = delete;

        void render() override;

    private:
        static constexpr int HISTORY_SIZE = 120;
        float                m_frame_time_history[HISTORY_SIZE] {};
        int                  m_history_index {0};
    };

} // namespace RealmEngine
