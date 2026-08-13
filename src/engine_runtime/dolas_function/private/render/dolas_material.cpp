#include "render/dolas_material.h"
#include "render/dolas_shader.h"
#include "dolas_engine.h"
#include "manager/dolas_texture_manager.h"
namespace Dolas
{
	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	std::shared_ptr<VertexContext> Material::GetVertexContext()
	{
		return m_vertex_context;
	}

	std::shared_ptr<PixelContext> Material::GetPixelContext()
	{
		return m_pixel_context;
	}
}