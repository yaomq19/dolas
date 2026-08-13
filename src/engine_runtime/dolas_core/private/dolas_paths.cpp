#include "dolas_paths.h"

namespace Dolas
{
#define SHADER_DIR_NAME "shader/"
	std::string PathUtils::g_engine_content_directory_path = ENGINE_CONTENT_DIR;
	std::string PathUtils::g_project_content_directory_path = "";

	void PathUtils::SetProjectDirectoryPath(const std::string& project_content_dir)
	{
		g_project_content_directory_path = project_content_dir;
	}

	//--------public begin
	std::string PathUtils::GetEngineContentDir()
	{
		return g_engine_content_directory_path;
	}

	std::string PathUtils::GetProjectContentDir()
	{
		return g_project_content_directory_path;
	}

	std::optional<std::filesystem::path> PathUtils::ResolveAssetPath(const AssetPath& asset_path)
	{
		const std::string& root_directory = asset_path.GetMount() == AssetMount::Engine
			? g_engine_content_directory_path
			: g_project_content_directory_path;

		if (root_directory.empty())
		{
			return std::nullopt;
		}

		const std::filesystem::path relative_path{std::string{asset_path.GetRelativePath()}};
		if (relative_path.empty() || relative_path.has_root_path())
		{
			return std::nullopt;
		}

		return (std::filesystem::path{root_directory} / relative_path).lexically_normal();
	}

	std::string PathUtils::GetShadersSourceDir() {
		return GetEngineContentDir() + SHADER_DIR_NAME;
	}

#if !defined(NDEBUG)
	void PathUtils::SetEngineContentDirForDebug(const std::string& engine_content_dir)
	{
		g_engine_content_directory_path = engine_content_dir;
	}

	void PathUtils::SetProjectContentDirForDebug(const std::string& project_content_dir)
	{
		g_project_content_directory_path = project_content_dir;
	}
#endif
	
} // namespace Dolas
