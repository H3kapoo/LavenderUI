#pragma once

#include "src/UIElements/UIBase.hpp"

namespace src::uielements
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

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;

protected:
};
using UIImagePtr = std::shared_ptr<UIImage>;
using UIImageWPtr = std::weak_ptr<UIImage>;
} // namespace src::uielements
