#include <catch2/catch_test_macros.hpp>

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <string>

#include "dolas_engine.h"
#include "dolas_render_hardware_interface.h"
#include "manager/dolas_render_view_manager.h"
#include "manager/dolas_tick_manager.h"
#include "render/dolas_render_view.h"

namespace
{
    template<typename ResourceType>
    using ComPtr = Microsoft::WRL::ComPtr<ResourceType>;

    class EngineCleanupGuard
    {
    public:
        void Enable()
        {
            m_should_clear = true;
        }

        ~EngineCleanupGuard()
        {
            if (m_should_clear)
            {
                Dolas::g_dolas_engine.Clear();
            }
        }

    private:
        bool m_should_clear = false;
    };

    bool HasVisibleWindowStation()
    {
        HWINSTA window_station = GetProcessWindowStation();
        if (!window_station)
        {
            return false;
        }

        USEROBJECTFLAGS user_object_flags {};
        DWORD bytes_needed = 0;
        if (!GetUserObjectInformationW(
                window_station,
                UOI_FLAGS,
                &user_object_flags,
                sizeof(user_object_flags),
                &bytes_needed))
        {
            return false;
        }

        return (user_object_flags.dwFlags & WSF_VISIBLE) != 0;
    }

    bool CanCreateHardwareD3D12Device(std::string& skip_reason)
    {
        ComPtr<IDXGIFactory6> factory;
        HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            skip_reason = "DXGI factory creation failed.";
            return false;
        }

        for (UINT adapter_index = 0; ; ++adapter_index)
        {
            ComPtr<IDXGIAdapter4> adapter;
            result = factory->EnumAdapterByGpuPreference(
                adapter_index,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&adapter));
            if (result == DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            if (FAILED(result))
            {
                continue;
            }

            DXGI_ADAPTER_DESC3 adapter_desc {};
            if (FAILED(adapter->GetDesc3(&adapter_desc)))
            {
                continue;
            }

            if ((adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                return true;
            }
        }

        skip_reason = "No hardware adapter can create a D3D12 device.";
        return false;
    }
}

TEST_CASE("Windows renderer initializes and renders one frame", "[render][smoke]")
{
    if (!HasVisibleWindowStation())
    {
        SKIP("No visible window station is available for the render smoke test.");
    }

    std::string skip_reason;
    if (!CanCreateHardwareD3D12Device(skip_reason))
    {
        SKIP(skip_reason);
    }

    EngineCleanupGuard engine_cleanup_guard;

    REQUIRE(Dolas::g_dolas_engine.Initialize());
    engine_cleanup_guard.Enable();

    REQUIRE(Dolas::g_dolas_engine.m_render_hardware_interface != nullptr);
    Dolas::RenderHardwareInterface* render_hardware_interface = Dolas::g_dolas_engine.m_render_hardware_interface;
    REQUIRE(render_hardware_interface->GetDevice() != nullptr);
    REQUIRE(render_hardware_interface->GetCommandQueue() != nullptr);
    REQUIRE(render_hardware_interface->GetSwapChain() != nullptr);
    REQUIRE(render_hardware_interface->GetWindowHandle() != nullptr);

    REQUIRE(Dolas::g_dolas_engine.m_render_view_manager != nullptr);
    REQUIRE(Dolas::g_dolas_engine.m_render_view_manager->GetMainRenderView() != nullptr);

    REQUIRE(Dolas::g_dolas_engine.m_tick_manager != nullptr);
    REQUIRE_NOTHROW(Dolas::g_dolas_engine.m_tick_manager->Tick(1.0f / 60.0f));

    REQUIRE(render_hardware_interface->GetDevice() != nullptr);
    REQUIRE(render_hardware_interface->GetCommandList() != nullptr);
    REQUIRE(render_hardware_interface->GetCurrentBackBufferResource() != nullptr);
}
