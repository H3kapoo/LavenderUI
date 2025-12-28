#pragma once

#include "src/Node/UIPane.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"

namespace lav::node
{
/**
    @brief
    Class holding a high amount of items in a performant way.
*/
class UIRecycleList : public UIPane
{
public:
    struct Model
    {
        virtual ~Model() = default;
        virtual auto makeAtIndex(UIButtonPtr& btn, const uint64_t index) -> void = 0;
        virtual auto getItemsCount() -> uint64_t = 0;
    };

public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIRecycleList, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIRecycleList);

    auto setModel(std::unique_ptr<Model> model) -> void;
    auto setRowSize(const uint32_t value) -> void;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(UIStatePtr& state) -> void override;

private:
    auto calculateLayout() -> glm::i64vec2;
    auto resolveVisibleItems() -> void;

private:
    std::unique_ptr<Model> model_{nullptr};
    uint32_t tolerance_{2};
    uint32_t rowSize_{16};
    int64_t topOfTheListIdx_{0};
    int64_t oldTopOfTheListIdx_{-1};
    int32_t visibleCount_{0};
    int32_t oldVisibleCount_{-1};
};
using UIRecycleListPtr = std::shared_ptr<UIRecycleList>;
using UIRecycleListWPtr = std::weak_ptr<UIRecycleList>;
} // namespace lav::node
