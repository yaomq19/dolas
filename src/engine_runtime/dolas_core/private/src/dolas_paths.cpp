#include "dolas_paths.h"
namespace Dolas
{
#define SHADER_DIR_NAME "shader/"
//--------public begin
	std::string PathUtils::GetContentDir()
	{
		return ENGINE_CONTENT_DIR;
	}

	std::string PathUtils::GetShadersSourceDir() {
		return GetContentDir() + SHADER_DIR_NAME;
	}
} // namespace Dolas
