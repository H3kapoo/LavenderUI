#pragma once

#include "src/Node/UIPane.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Utils/Logger.hpp"

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
        virtual auto getDataIndexFromVisualIndex(const uint64_t) -> uint64_t = 0;
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

            inOutItem->setText(std::to_string(data_[getDataIndexFromVisualIndex(index)]));
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

        virtual auto getDataIndexFromVisualIndex(const uint64_t idx) -> uint64_t { return idx; }
        virtual auto getItemsCount() -> uint64_t { return data_.size(); }
    };

    struct FilteredModel : BasicModel
    {
        FilteredModel(const std::vector<uint64_t>& data,
            const std::function<bool(uint64_t)> pred)
            : BasicModel(data)
            , pred_(pred)
            , mapping_()
        {
            mapping_.reserve(data_.size());
            for (uint64_t dIdx = 0; dIdx < data_.size(); dIdx++)
            {
                if (!pred(data_[dIdx])) { continue; }
                mapping_.push_back(dIdx);

                //               x
                // idx   0 1 2 3 4 5 6
                // data  1 2 3 4 5 6 7
                // mapp  0 2 4

                // data_[map[index]]
            }
        }

        auto getDataIndexFromVisualIndex(const uint64_t idx) -> uint64_t
        {
            return mapping_[idx];
        }

        auto getItemsCount() -> uint64_t { return mapping_.size(); }

        const std::function<bool(uint64_t)> pred_;
        std::vector<uint64_t> mapping_;
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
