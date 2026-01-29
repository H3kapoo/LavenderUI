#pragma once

#include <LavenderUI/Core/ResourceHandler/Texture.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
/**
    @brief
    Class representing an UI element for displaying images.
*/
class UIImage : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIImage, "elemVert.glsl", "elemFrag.glsl");

    auto setImage(const std::filesystem::path& path) -> bool;

protected:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(core::UIStatePtr& state) -> void override;

    INSERT_ADD_REMOVE_NOT_ALLOWED(UImage);

private:
    core::Texture imgTexData_;
};
using UIImagePtr = std::shared_ptr<UIImage>;
using UIImageWPtr = std::weak_ptr<UIImage>;
} // namespace lav::node
