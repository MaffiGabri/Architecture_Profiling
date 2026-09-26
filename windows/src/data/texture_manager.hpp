#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <iostream>
#include "architecture/models.hpp"
#include "imgui.h"

namespace arch::platform {

struct TextureResource {
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv{nullptr};
    int width{0};
    int height{0};
    int channels{0};

    [[nodiscard]] bool is_valid() const noexcept { return srv != nullptr; }

    [[nodiscard]] ImTextureID texture_id() const noexcept {
        return reinterpret_cast<ImTextureID>(srv.Get());
    }

    [[nodiscard]] float aspect_ratio() const noexcept {
        return (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : (4.0f / 3.0f);
    }
};

class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager() { shutdown(); }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) noexcept = default;
    TextureManager& operator=(TextureManager&&) noexcept = default;

    /**
     * Initializes the texture manager with the active DirectX 11 device.
     * Generates a 2x2 neutral fallback placeholder texture.
     */
    bool initialize(ID3D11Device* device, const std::filesystem::path& asset_root = "");

    /**
     * Pre-warms/loads the 10 mock style PNG images from assets/images.
     */
    bool prewarm_style_images(const std::vector<arch::domain::Style>& styles);

    /**
     * Retrieves the cached texture for a given style ID.
     * Returns the fallback placeholder texture if the style image is not found.
     */
    [[nodiscard]] const TextureResource& get_style_texture(int style_id) const noexcept;

    /**
     * Loads a texture directly from disk and uploads it to DirectX 11 VRAM.
     */
    TextureResource load_texture_from_file(const std::filesystem::path& file_path);

    /**
     * Releases all cached shader resource views and GPU textures.
     */
    void shutdown() noexcept;

    /**
     * Resolves an asset file path checking local, executable, and shared paths.
     */
    [[nodiscard]] std::filesystem::path resolve_image_path(const std::string& filename) const;

    [[nodiscard]] size_t loaded_count() const noexcept { return m_style_textures.size(); }

private:
    bool create_fallback_texture();

    Microsoft::WRL::ComPtr<ID3D11Device> m_device{nullptr};
    std::filesystem::path m_asset_root;
    std::unordered_map<int, TextureResource> m_style_textures;
    TextureResource m_fallback_texture;
};

} // namespace arch::platform
