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
		std::string prefix;
		std::string base_path;

		if (relative_path.starts_with("_engine"))
		{
			prefix = "_engine";
			base_path = g_engine_content_directory_path;
		}
		else if (relative_path.starts_with("_project"))
		{
			if (g_project_content_directory_path.empty())
			{
				return std::nullopt; // Project content directory not set
			}
			prefix = "_project";
			base_path = g_project_content_directory_path;
		}
		else
		{
			return std::nullopt; // Invalid prefix
		}

		std::string sub_path = relative_path.substr(prefix.length());
		
		// 统一处理 base_path 后缀：确保它以斜杠结尾
		if (!base_path.empty() && base_path.back() != '/' && base_path.back() != '\\')
		{
			base_path += "/";
		}

		// 统一处理 sub_path 前缀：如果是以斜杠开头，去掉它（因为 base_path 已经保证有斜杠了）
		if (!sub_path.empty() && (sub_path.front() == '/' || sub_path.front() == '\\'))
		{
			sub_path = sub_path.substr(1);
		}

		return base_path + sub_path;
	}

	std::string PathUtils::GetShadersSourceDir() {
		return GetEngineContentDir() + SHADER_DIR_NAME;
	}

#if defined(DEBUG) | defined(_DEBUG)
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
