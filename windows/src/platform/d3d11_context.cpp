#include "d3d11_context.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace arch::platform {

namespace {
D3D11Context* g_context_instance = nullptr;

LRESULT WINAPI StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_context_instance) {
        return g_context_instance->handle_message(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
} // namespace

bool D3D11Context::initialize(int width, int height, const std::wstring& title) {
    g_context_instance = this;

    // Enable High DPI Awareness
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Register Win32 Window Class
    ZeroMemory(&m_wc, sizeof(m_wc));
    m_wc.cbSize = sizeof(WNDCLASSEXW);
    m_wc.style = CS_CLASSDC;
    m_wc.lpfnWndProc = StaticWndProc;
    m_wc.hInstance = GetModuleHandle(nullptr);
    m_wc.lpszClassName = L"ArchitectureProfilingWin32Class";
    m_wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassExW(&m_wc)) {
        std::cerr << "[D3D11Context] Failed to register window class." << std::endl;
        return false;
    }

    // Determine window position centered on screen
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int x_pos = (screen_w > width) ? (screen_w - width) / 2 : 50;
    int y_pos = (screen_h > height) ? (screen_h - height) / 2 : 50;

    m_hwnd = CreateWindowW(
        m_wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        x_pos, y_pos,
        width, height,
        nullptr, nullptr,
        m_wc.hInstance,
        nullptr
    );

    if (!m_hwnd) {
        std::cerr << "[D3D11Context] Failed to create Win32 window." << std::endl;
        UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        return false;
    }

    // Initialize DirectX 11 device and swapchain
    if (!create_device_and_swapchain(width, height)) {
        std::cerr << "[D3D11Context] Failed to create D3D11 device and swapchain." << std::endl;
        DestroyWindow(m_hwnd);
        UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        return false;
    }

    // Create render target view
    if (!create_render_target()) {
        std::cerr << "[D3D11Context] Failed to create render target view." << std::endl;
        shutdown();
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    // Initialize Dear ImGui context and backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        std::cerr << "[D3D11Context] ImGui_ImplWin32_Init failed." << std::endl;
        shutdown();
        return false;
    }

    if (!ImGui_ImplDX11_Init(m_device.Get(), m_device_context.Get())) {
        std::cerr << "[D3D11Context] ImGui_ImplDX11_Init failed." << std::endl;
        shutdown();
        return false;
    }

    m_running = true;
    return true;
}

bool D3D11Context::create_device_and_swapchain(int width, int height) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = static_cast<UINT>(width);
    sd.BufferDesc.Height = static_cast<UINT>(height);
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    constexpr D3D_FEATURE_LEVEL featureLevels[3] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        3,
        D3D11_SDK_VERSION,
        &sd,
        m_swap_chain.GetAddressOf(),
        m_device.GetAddressOf(),
        &featureLevel,
        m_device_context.GetAddressOf()
    );

    if (FAILED(hr)) {
        // Fallback to WARP (software rasterizer) if hardware adapter creation fails
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createDeviceFlags,
            featureLevels,
            3,
            D3D11_SDK_VERSION,
            &sd,
            m_swap_chain.GetAddressOf(),
            m_device.GetAddressOf(),
            &featureLevel,
            m_device_context.GetAddressOf()
        );
        if (FAILED(hr)) return false;
    }

    return true;
}

bool D3D11Context::create_render_target() {
    if (!m_swap_chain || !m_device) return false;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    HRESULT hr = m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = m_device->CreateRenderTargetView(
        back_buffer.Get(),
        nullptr,
        m_main_render_target_view.GetAddressOf()
    );
    return SUCCEEDED(hr);
}

void D3D11Context::cleanup_render_target() {
    if (m_main_render_target_view) {
        m_main_render_target_view.Reset();
    }
}

void D3D11Context::resize_buffers(UINT width, UINT height) {
    if (!m_swap_chain) return;

    cleanup_render_target();
    HRESULT hr = m_swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        create_render_target();
    }
}

bool D3D11Context::process_messages() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT) {
            m_running = false;
            return false;
        }
    }
    return m_running;
}

void D3D11Context::begin_frame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void D3D11Context::end_frame(bool vsync, float clear_color[4]) {
    ImGui::Render();

    float default_clear[4] = {0.059f, 0.067f, 0.090f, 1.0f}; // Basalt Dark
    float* col = clear_color ? clear_color : default_clear;

    if (m_main_render_target_view && m_device_context) {
        ID3D11RenderTargetView* rtv = m_main_render_target_view.Get();
        m_device_context->OMSetRenderTargets(1, &rtv, nullptr);
        m_device_context->ClearRenderTargetView(rtv, col);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    if (m_swap_chain) {
        HRESULT hr = m_swap_chain->Present(vsync ? 1 : 0, 0);
        m_swapchain_occluded = (hr == DXGI_STATUS_OCCLUDED);
    }
}

void D3D11Context::shutdown() {
    m_running = false;

    // Shutdown ImGui backends
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    if (ImGui::GetCurrentContext()) {
        ImGui::DestroyContext();
    }

    cleanup_render_target();

    m_swap_chain.Reset();
    m_device_context.Reset();
    m_device.Reset();

    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_wc.lpszClassName) {
        UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        m_wc.lpszClassName = nullptr;
    }

    g_context_instance = nullptr;
}

LRESULT D3D11Context::handle_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) return 0;
            resize_buffers(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) {
                // Disable ALT menu freeze
                return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace arch::platform
