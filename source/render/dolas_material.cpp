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

	VertexShader* Material::GetVertexShader()
	{
		return m_vertex_shader;
	}

	PixelShader* Material::GetPixelShader()
	{
		return m_pixel_shader;
	}


	Bool Material::BindPixelShaderTextures()
	{
		for (auto tex_slot_id_pair : m_pixel_shader_textures)
		{
			int slot = tex_slot_id_pair.first;
			TextureID texture_id = tex_slot_id_pair.second;
			Texture* tex = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
			DOLAS_CONTINUE_IF_NULL(tex);
			m_pixel_shader->SetShaderResourceView(slot, tex->GetShaderResourceView());

		}
		return true;
	}
}