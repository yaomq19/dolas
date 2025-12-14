using System;
using System.IO;

namespace Dolas.AssetEditor.Services;

public static class RepoLocator
{
    /// <summary>
    /// 从起始目录向上查找，直到找到包含 content/ 的仓库根目录。
    /// </summary>
    public static string? TryFindRepoRoot(string startDir, int maxDepth = 12)
    {
        var dir = new DirectoryInfo(startDir);
        for (var i = 0; i < maxDepth && dir is not null; i++)
        {
            var contentDir = Path.Combine(dir.FullName, "content");
            var cmakeFile = Path.Combine(dir.FullName, "CMakeLists.txt");
            if (Directory.Exists(contentDir) && File.Exists(cmakeFile))
                return dir.FullName;

            dir = dir.Parent;
        }

        return null;
    }

    public static string? TryFindContentRoot()
    {
        var baseDir = AppContext.BaseDirectory;
        var repo = TryFindRepoRoot(baseDir);
        return repo is null ? null : Path.Combine(repo, "content");
    }
}


