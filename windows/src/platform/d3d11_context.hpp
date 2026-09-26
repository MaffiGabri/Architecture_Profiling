#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace arch::platform {

class D3D11Context {
public:
    D3D11Context() = default;
    ~D3D11Context() { shutdown(); }

    D3D11Context(const D3D11Context&) = delete;
    D3D11Context& operator=(const D3D11Context&) = delete;

    /**
     * Initializes Win32 window, DirectX 11 device, swapchain,
     * render target view, and Dear ImGui Win32 & DX11 backends.
     */
    bool initialize(int width, int height, const std::wstring& title);

    /**
     * Dispatches queued Win32 messages.
     * Returns false when WM_QUIT is received.
     */
    bool process_messages();

    /**
     * Begins an ImGui frame.
     */
    void begin_frame();

    /**
     * Ends the ImGui frame, renders draw data, clears background,
     * and presents the swapchain with VSync.
     */
    void end_frame(bool vsync = true, float clear_color[4] = nullptr);

    /**
     * Shuts down ImGui, releases DirectX 11 objects, and destroys the window.
     */
    void shutdown();

    [[nodiscard]] HWND hwnd() const noexcept { return m_hwnd; }
    [[nodiscard]] ID3D11Device* device() const noexcept { return m_device.Get(); }
    [[nodiscard]] ID3D11DeviceContext* device_context() const noexcept { return m_device_context.Get(); }
    [[nodiscard]] bool is_running() const noexcept { return m_running; }

    // Internal message handler helper
    LRESULT handle_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool create_device_and_swapchain(int width, int height);
    bool create_render_target();
    void cleanup_render_target();
    void resize_buffers(UINT width, UINT height);

    HWND m_hwnd{nullptr};
    WNDCLASSEXW m_wc{};
    bool m_running{false};
    bool m_swapchain_occluded{false};

    Microsoft::WRL::ComPtr<ID3D11Device> m_device{nullptr};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_device_context{nullptr};
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swap_chain{nullptr};
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_main_render_target_view{nullptr};
};

} // namespace arch::platform
