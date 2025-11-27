#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include "editor/widget.h"

namespace RealmEngine
{
    class FileDialogWidget : public Widget
    {
    public:
        enum class Mode
        {
            Open,
            Save
        };

        using OnFileSelectedCallback = std::function<void(const std::filesystem::path&)>;

        FileDialogWidget();
        ~FileDialogWidget() override = default;

        FileDialogWidget(const FileDialogWidget&)            = delete;
        FileDialogWidget& operator=(const FileDialogWidget&) = delete;
        FileDialogWidget(FileDialogWidget&&)                 = default;
        FileDialogWidget& operator=(FileDialogWidget&&)      = default;

        void render() override;

        void open(Mode                         mode,
                  const std::string&           title,
                  const std::string&           filter,
                  const std::filesystem::path& initial_path = "");
        void close();
        bool isOpen() const { return m_open && m_dialog_open; }

        void setOnFileSelected(OnFileSelectedCallback callback) { m_callback = callback; }
        Mode getMode() const { return m_mode; }

    private:
        void renderDialog();
        void navigateTo(const std::filesystem::path& path);
        void updateCurrentPath();

        bool                               m_dialog_open {false};
        Mode                               m_mode {Mode::Open};
        std::string                        m_title;
        std::string                        m_filter;
        std::filesystem::path              m_current_path;
        std::filesystem::path              m_selected_path;
        std::vector<std::filesystem::path> m_current_directory_entries;
        char                               m_filename_buffer[256] {0};
        OnFileSelectedCallback             m_callback;
    };

} // namespace RealmEngine
