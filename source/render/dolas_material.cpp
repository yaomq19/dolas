#include "render/dolas_material.h"

namespace Dolas
{
	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	bool Material::BuildFromFile(const std::string& file_path)
	{
		return true;
	}

	std::string Material::GetFilePath()
	{
		return m_file_path;
	}

	VertexShader* Material::GetVertexShader()
	{
		return m_vertex_shader;
	}

	PixelShader* Material::GetPixelShader()
	{
		return m_pixel_shader;
	}

	std::vector<std::pair<int, std::string>> Material::GetTextures()
	{
		return m_textures;
	}
}