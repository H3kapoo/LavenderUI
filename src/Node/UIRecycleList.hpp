#pragma once

#include "src/Node/UIPane.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"

namespace lav::node
{
/**
    @brief
    Class used to display and cycle around many list items in a performant lazy way.
*/
class UIRecycleList : public UIPane
{
public:
    struct AbstractModel
    {
        virtual ~AbstractModel() = default;
        virtual auto dataForIndex(UIButtonPtr&, const uint64_t) -> void = 0;
        virtual auto getItemsCount() -> uint64_t = 0;
    };

    struct BasicModel : AbstractModel
    {
        BasicModel(const std::vector<uint64_t>& data)
            : data_(data)
        {}

        const std::vector<uint64_t>& data_;

        auto dataForIndex(UIButtonPtr& inOutItem, const uint64_t index) -> void
        {
            if (index >= data_.size()) { return; }

            inOutItem->setText(std::to_string(data_[index]));
            inOutItem->setColor(index % 2
                ? utils::hexToVec4("#adadadff")
                : utils::hexToVec4("#e46b6bff"));
            inOutItem->listenEvent<core::MouseLeftReleaseEvt>(
                [this, index](const auto& e)
                {
                    (void)e;
                    utils::Logger log{"InsideBtn"};
                    log.debug("clicked on {} data {}", index, data_[index]);
                });
        }

        auto getItemsCount() -> uint64_t { return data_.size(); }
    };

public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIRecycleList, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIRecycleList);

    auto setModel(std::unique_ptr<AbstractModel> model) -> void;
    auto setRowSize(const uint32_t value) -> void;

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(UIStatePtr& state) -> void override;

private:
    auto calculateLayout() -> glm::i64vec2;
    auto resolveVisibleItems() -> void;

private:
    std::unique_ptr<AbstractModel> model_{nullptr};
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
