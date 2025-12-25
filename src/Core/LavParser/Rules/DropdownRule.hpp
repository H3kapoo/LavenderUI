#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIDropdown.hpp"

namespace lav::core
{
class DropdownRule : public IRule
{
public:
    auto construct(node::UIBasePtr parent,
        const hk::XMLDecoder::NodeSPtr& xmlNode) -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const hk::XMLDecoder::AttrPairVec& attribs) -> void override;

private:
    auto toOpenDir(const std::string& value) const -> node::UIDropdown::OpenDir;
};
} // namespace lav::core