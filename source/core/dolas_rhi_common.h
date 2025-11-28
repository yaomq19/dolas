#ifndef DOLAS_RHI_COMMON_H
#define DOLAS_RHI_COMMON_H
#include <d3d11.h>
#include <d3d11_1.h>
#include "common/dolas_hash.h"
#include "core/dolas_math.h"

namespace Dolas
{
	class RenderTargetView
	{
	public:
		RenderTargetView();
		~RenderTargetView();

		ID3D11RenderTargetView* m_d3d_render_target_view;
	};

	class DepthStencilView
	{
	public:
		DepthStencilView();
		~DepthStencilView();

		ID3D11DepthStencilView* m_d3d_depth_stencil_view;
	};

	class ViewPort
	{
	public:
		ViewPort(Float top_left_x, Float top_left_y, Float width, Float height, Float min_depth, Float max_depth);
		~ViewPort();
		D3D11_VIEWPORT m_d3d_viewport;
	};

	enum RasterizerStateType : UInt
	{
		SolidNoneCull,
		SolidBackCull,
		SolidFrontCull,
		Wireframe,
		RasterizerStateCount,
	};

	class RasterizerState
	{
	public:
		RasterizerState();
		~RasterizerState();

		ID3D11RasterizerState* m_d3d_rasterizer_state;
	};

	enum DepthStencilStateType : UInt
	{
		DepthEnabled,
		DepthDisabled,
		DepthReadOnly,
		DepthStencilStateCount,
	};

	class DepthStencilState
	{
	public:
		DepthStencilState();
		~DepthStencilState();

		ID3D11DepthStencilState* m_d3d_depth_stencil_state;
	};

	enum BlendStateType : UInt
	{
		Opaque,
		AlphaBlend,
		Additive,
		BlendStateCount,
	};

	class BlendState
	{
	public:
		BlendState();
		~BlendState();

		ID3D11BlendState* m_d3d_blend_state;
	};

	struct PerViewConstantBuffer
	{
		Matrix4x4 view;
		Matrix4x4 proj;
		Vector4 camera_position; // w is unused
	};

	struct PerObjectConstantBuffer
	{
		Matrix4x4 world;
	};

	struct PerFrameConstantBuffer
	{
		Vector4 light_direction_intensity;
		Vector4 light_color; // w is unused
	};

	struct DepthClearParams
	{
		Bool enable = true;
		Float clear_value = 1.0f;
	};

	struct StencilClearParams
	{
		Bool enable = true;
		UInt clear_value = 0;
	};

	enum PrimitiveTopology : UInt
	{
		PrimitiveTopology_TriangleList,
		PrimitiveTopology_Count
	};

	enum InputLayoutType
	{
		InputLayoutType_POS_3,
		InputLayoutType_POS_3_UV_2,
		InputLayoutType_POS_3_UV_2_NORM_3,
		InputLayoutType_Count
	};

	class InputLayout
	{
	public:
		InputLayout();
		~InputLayout();

		ID3D11InputLayout* m_d3d_input_layout;
	};
}

#endif