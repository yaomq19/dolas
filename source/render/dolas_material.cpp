#include "render/dolas_material.h"
#include "render/dolas_shader.h"
#include "core/dolas_engine.h"
#include "manager/dolas_texture_manager.h"
namespace Dolas
{
	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	VertexContext* Material::GetVertexContext()
	{
		return m_vertex_context;
	}

	PixelContext* Material::GetPixelContext()
	{
		return m_pixel_context;
	}
}