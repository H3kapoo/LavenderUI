#pragma once

#include "src/Core/ResourceHandler/Texture.hpp"
#include "src/Node/UIBase.hpp"

namespace lav::node
{
class UIImage : public UIBase
{
public:
    UIImage();
    ~UIImage() = default;
    UIImage(const UIImage&) = delete;
    UIImage(UIBase&&) = delete;
    UIImage& operator=(const UIImage&) = delete;
    UIImage& operator=(UIImage&&) = delete;

    auto setImage(const std::filesystem::path& path) -> bool;

    /* Mandatory define */
    INSERT_TYPEINFO(UIImage);

protected:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(UIStatePtr& state) -> void override;

private:
    core::Texture imgTexData_;
};
using UIImagePtr = std::shared_ptr<UIImage>;
using UIImageWPtr = std::weak_ptr<UIImage>;
} // namespace lav::node
