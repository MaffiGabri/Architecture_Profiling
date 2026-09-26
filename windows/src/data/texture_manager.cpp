#include "texture_manager.hpp"
#include <windows.h>
#include <iostream>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO_NOT_REQUIRED
#include "stb/stb_image.h"

namespace arch::platform {

bool TextureManager::initialize(ID3D11Device* device, const std::filesystem::path& asset_root) {
    if (!device) return false;
    m_device = device;
    m_asset_root = asset_root;
    return create_fallback_texture();
}

std::filesystem::path TextureManager::resolve_image_path(const std::string& filename) const {
    // 1. Direct explicit asset root
    if (!m_asset_root.empty()) {
        auto p1 = m_asset_root / "images" / filename;
        if (std::filesystem::exists(p1)) return p1;
        auto p2 = m_asset_root / filename;
        if (std::filesystem::exists(p2)) return p2;
    }

    // 2. Executable-relative path via GetModuleFileNameW
    wchar_t exe_buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exe_buf, MAX_PATH) > 0) {
        std::filesystem::path exe_dir = std::filesystem::path(exe_buf).parent_path();
        auto p1 = exe_dir / "assets" / "images" / filename;
        if (std::filesystem::exists(p1)) return p1;
        auto p2 = exe_dir / "images" / filename;
        if (std::filesystem::exists(p2)) return p2;
        auto p3 = exe_dir.parent_path() / "shared" / "images" / filename;
        if (std::filesystem::exists(p3)) return p3;
    }

    // 3. Local working directory relative paths
    std::array<std::filesystem::path, 7> candidates = {
        std::filesystem::path("assets/images") / filename,
        std::filesystem::path("images") / filename,
        std::filesystem::path("shared/images") / filename,
        std::filesystem::path("../shared/images") / filename,
        std::filesystem::path("../../shared/images") / filename,
        std::filesystem::path("../../../shared/images") / filename,
        std::filesystem::path(filename)
    };

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return filename; // Fallback to raw filename
}

bool TextureManager::create_fallback_texture() {
    if (!m_device) return false;

    // 2x2 Slate Gray (#333842) neutral RGBA8 placeholder
    constexpr uint32_t fallback_pixels[4] = {
        0xFF423833, 0xFF4A403A,
        0xFF4A403A, 0xFF423833
    };

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 2;
    desc.Height = 2;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init_data{};
    init_data.pSysMem = fallback_pixels;
    init_data.SysMemPitch = 2 * sizeof(uint32_t);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&desc, &init_data, tex.GetAddressOf());
    if (FAILED(hr)) {
        std::cerr << "[TextureManager] Error: Failed to create fallback texture 2D (0x" 
                  << std::hex << hr << ")" << std::endl;
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(tex.Get(), &srv_desc, m_fallback_texture.srv.GetAddressOf());
    if (FAILED(hr)) {
        std::cerr << "[TextureManager] Error: Failed to create fallback SRV (0x" 
                  << std::hex << hr << ")" << std::endl;
        return false;
    }

    m_fallback_texture.width = 2;
    m_fallback_texture.height = 2;
    m_fallback_texture.channels = 4;
    return true;
}

TextureResource TextureManager::load_texture_from_file(const std::filesystem::path& file_path) {
    if (!m_device) return m_fallback_texture;

    int w = 0, h = 0, ch = 0;
    // Force 4 channels (RGBA)
    stbi_uc* pixels = stbi_load(file_path.string().c_str(), &w, &h, &ch, 4);
    if (!pixels) {
        std::cerr << "[TextureManager] Warning: Failed to load image " << file_path << std::endl;
        return m_fallback_texture;
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(w);
    desc.Height = static_cast<UINT>(h);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init_data{};
    init_data.pSysMem = pixels;
    init_data.SysMemPitch = static_cast<UINT>(w * 4);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&desc, &init_data, tex.GetAddressOf());
    stbi_image_free(pixels); // Free CPU memory buffer immediately

    if (FAILED(hr)) {
        std::cerr << "[TextureManager] Error: CreateTexture2D failed for " << file_path 
                  << " (0x" << std::hex << hr << ")" << std::endl;
        return m_fallback_texture;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    TextureResource res;
    hr = m_device->CreateShaderResourceView(tex.Get(), &srv_desc, res.srv.GetAddressOf());
    if (FAILED(hr)) {
        std::cerr << "[TextureManager] Error: CreateShaderResourceView failed for " << file_path 
                  << " (0x" << std::hex << hr << ")" << std::endl;
        return m_fallback_texture;
    }

    res.width = w;
    res.height = h;
    res.channels = 4;
    return res;
}

bool TextureManager::prewarm_style_images(const std::vector<arch::domain::Style>& styles) {
    if (!m_device) return false;
    for (const auto& style : styles) {
        auto path = resolve_image_path(style.filename);
        TextureResource tex = load_texture_from_file(path);
        m_style_textures[style.id] = std::move(tex);
    }
    return true;
}

const TextureResource& TextureManager::get_style_texture(int style_id) const noexcept {
    auto it = m_style_textures.find(style_id);
    if (it != m_style_textures.end() && it->second.is_valid()) {
        return it->second;
    }
    return m_fallback_texture;
}

void TextureManager::shutdown() noexcept {
    m_style_textures.clear();
    m_fallback_texture.srv.Reset();
    m_device.Reset();
}

} // namespace arch::platform
