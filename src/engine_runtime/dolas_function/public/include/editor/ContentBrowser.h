#ifndef DOLAS_CONTENT_BROWSER_H
#define DOLAS_CONTENT_BROWSER_H

#include "dolas_base.h"
#include <string>
#include <vector>

struct ImVec4;

namespace Dolas
{
    enum class FileType : UInt
    {
        Folder,
        Mesh,
        Texture,
        Material,
        Shader,
        Scene,
        Camera,
        Unknown
    };

    struct FileInfo
    {
        std::string name;
        std::string path;
        FileType type;
        ULongLong size;
        std::string modified_time;
        bool is_directory;
    };

    class ContentBrowser
    {
    public:
        ContentBrowser();
        ~ContentBrowser();

        void Render();
        void Refresh();

        std::string GetSelectedPath() const;

    private:
        void RenderDirectoryTree();
        void RenderFileList();
        void RenderToolbar();
        void RenderPreviewPanel();

        void ScanDirectory(const std::string& path);
        FileType GetFileType(const std::string& extension) const;
        const char* GetFileTypeIcon(FileType type) const;
        const ImVec4& GetFileTypeColor(FileType type) const;

        bool NavigateTo(const std::string& path);
        void NavigateBack();
        void NavigateForward();
        void NavigateUp();

        std::string m_base_path;
        std::string m_current_path;
        std::string m_selected_path;

        std::vector<FileInfo> m_files;
        std::vector<std::string> m_nav_history;
        size_t m_nav_index;

        bool m_grid_view;
        float m_icon_size;
        float m_spacing;
    };
}

#endif // DOLAS_CONTENT_BROWSER_H
