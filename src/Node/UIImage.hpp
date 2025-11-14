#pragma once

#include "src/Core/ResourceHandler/Texture.hpp"
#include "src/Node/UIBase.hpp"

namespace lav::node
{

class UIImage : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIImage, "elemVert.glsl", "elemFrag.glsl");

    auto setImage(const std::filesystem::path& path) -> bool;

protected:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(UIStatePtr& state) -> void override;

    INSERT_ADD_REMOVE_NOT_ALLOWED(UImage);

private:
    core::Texture imgTexData_;
};
using UIImagePtr = std::shared_ptr<UIImage>;
using UIImageWPtr = std::weak_ptr<UIImage>;
} // namespace lav::node
