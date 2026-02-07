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

	std::optional<std::string> PathUtils::CombineToFullPath(const std::string& relative_path)
	{
		if (relative_path.starts_with("_engine/"))
		{
			return g_engine_content_directory_path + relative_path.substr(std::string("_engine/").length());
		}
		else if (relative_path.starts_with("_project/"))
		{
			if (g_project_content_directory_path.empty())
			{
				return std::nullopt; // Project content directory not set
			}
			return g_project_content_directory_path + relative_path.substr(std::string("_project/").length());
		}
		return std::nullopt; // Invalid prefix
	}

	std::string PathUtils::GetShadersSourceDir() {
		return GetEngineContentDir() + SHADER_DIR_NAME;
	}
} // namespace Dolas
