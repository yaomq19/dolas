#include "render/dolas_material.h"

namespace Dolas
{
	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	VertexShader* Material::GetVertexShader()
	{
		return m_vertex_shader;
	}

	PixelShader* Material::GetPixelShader()
	{
		return m_pixel_shader;
	}

}