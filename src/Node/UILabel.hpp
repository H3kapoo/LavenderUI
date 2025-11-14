#pragma once

#include <optional>

#include "src/Core/TextHandler/TextAttribs.hpp"
#include "src/Node/UIBase.hpp"

namespace lav::node
{
/**
    @brief Label element for displaying text.
*/
class UILabel : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UILabel, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UILabel);

    auto setText(const std::string& text) -> UILabel&;
    auto setFont(const std::filesystem::path& fontPath) -> void;

private:
    virtual auto render(const glm::mat4& projection) -> void override;
    virtual auto layout() -> void override;
    virtual auto event(UIStatePtr& state) -> void override;

protected:
    core::TextAttribs textAttribs_;
    std::optional<glm::vec4> overrideColor_{std::nullopt};
};
using UILabelPtr = std::shared_ptr<UILabel>;
using UILabelWPtr = std::weak_ptr<UILabel>;
using UILabelPtrVec = std::vector<UILabelPtr>;
} // namespace lav::node
