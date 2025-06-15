#include "dolas_paths.h"
namespace Dolas
{
#define CONTENT_DIR_NAME "content/"
#define SHADER_DIR_NAME "shader/"
#define ENTITY_DIR_NAME "entity/"
#define MESH_DIR_NAME "mesh/"
#define MATERIAL_DIR_NAME "material/"
#define TEXTURE_DIR_NAME "texture/"

//--------public begin
	std::wstring PathUtils::StringToWString(const std::string& str)
	{
		std::wstring wstrTo(str.begin(), str.end());
		return wstrTo;
	}

	std::string PathUtils::GetContentDir()
	{
		return GetProjectRoot() + CONTENT_DIR_NAME;
	}

	std::string PathUtils::GetShadersSourceDir() {
		return GetContentDir() + SHADER_DIR_NAME;
	}

	std::string PathUtils::GetShadersCompiledDir()
	{
		return GetBinDir() + SHADER_DIR_NAME;
	}

	std::string PathUtils::GetEntityDir()
	{
		return GetContentDir() + ENTITY_DIR_NAME;
	}

	std::string PathUtils::GetMeshDir()
	{
		return GetContentDir() + MESH_DIR_NAME;
	}

	std::string PathUtils::GetMaterialDir()
	{
		return GetContentDir() + MATERIAL_DIR_NAME;
	}

	std::string PathUtils::GetTextureDir()
	{
		return GetContentDir() + TEXTURE_DIR_NAME;
	}
//--------public end

//--------private begin
	// 获取项目根目录路径
	std::string PathUtils::GetProjectRoot()
	{
		return std::string(CMAKE_SOURCE_DIR);
	}

	// 获取项目 exe 文件所在路径
	std::string PathUtils::GetBinDir() {
		return std::string(CMAKE_RUNTIME_OUTPUT_DIRECTORY);
	}

//--------private end
} // namespace Dolas
