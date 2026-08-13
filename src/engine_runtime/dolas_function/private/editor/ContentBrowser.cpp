#include "editor/ContentBrowser.h"
#include <imgui.h>
#include <filesystem>

namespace Dolas
{
    ContentBrowser::ContentBrowser()
        : m_base_path("content/")
        , m_current_path(m_base_path)
        , m_selected_path("")
        , m_nav_index(0)
        , m_grid_view(true)
        , m_icon_size(80.0f)
        , m_spacing(10.0f)
    {
        m_nav_history.push_back(m_current_path);
        Refresh();
    }

    ContentBrowser::~ContentBrowser()
    {
    }

    void ContentBrowser::Render()
    {
        RenderToolbar();

        ImGui::Separator();

        ImGui::BeginChild("DirectoryTree", ImVec2(150, 0), true);
        RenderDirectoryTree();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Files", ImVec2(0, 0), true);
        RenderFileList();
        ImGui::EndChild();

        RenderPreviewPanel();
    }

    void ContentBrowser::Refresh()
    {
        m_files.clear();
        ScanDirectory(m_current_path);
    }

    std::string ContentBrowser::GetSelectedPath() const
    {
        return m_selected_path;
    }

    void ContentBrowser::RenderDirectoryTree()
    {
        if (ImGui::TreeNode("content/"))
        {
            if (ImGui::Selectable("scene/"))
            {
                NavigateTo("content/scene/");
            }
            ImGui::TreePop();
        }
    }

    void ContentBrowser::RenderFileList()
    {
    }

    void ContentBrowser::RenderToolbar()
    {
    }

    void ContentBrowser::RenderPreviewPanel()
    {
    }

    void ContentBrowser::ScanDirectory(const std::string& path)
    {
    }

    FileType ContentBrowser::GetFileType(const std::string& extension) const
    {
        return FileType::Unknown;
    }

    const char* ContentBrowser::GetFileTypeIcon(FileType type) const
    {
        return "";
    }

    const ImVec4& ContentBrowser::GetFileTypeColor(FileType type) const
    {
        static ImVec4 s_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        return s_color;
    }

    bool ContentBrowser::NavigateTo(const std::string& path)
    {
        m_current_path = path;
        if (m_nav_index < m_nav_history.size() - 1)
        {
            m_nav_history.resize(m_nav_index + 1);
        }
        m_nav_history.push_back(path);
        m_nav_index = m_nav_history.size() - 1;
        Refresh();
        return true;
    }

    void ContentBrowser::NavigateBack()
    {
        if (m_nav_index > 0)
        {
            m_nav_index--;
            m_current_path = m_nav_history[m_nav_index];
            Refresh();
        }
    }

    void ContentBrowser::NavigateForward()
    {
        if (m_nav_index < m_nav_history.size() - 1)
        {
            m_nav_index++;
            m_current_path = m_nav_history[m_nav_index];
            Refresh();
        }
    }

    void ContentBrowser::NavigateUp()
    {
    }
}
