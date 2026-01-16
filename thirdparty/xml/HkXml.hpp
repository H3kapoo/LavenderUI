#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace hk
{

/* DISCLAIMER:
    - very basic error checks and notifiers
    - ![CDATA[ not supported
    - < > ? / are allowed inside tag values
    - " character is not escaped inside tag values
    - <? processing information ?> not supported
    - Sure, it can be way more optimized but this wasn't the goal.
    - Except for a bit of back and forth, for all intents and purposes the algorithm is O(N)
      and the data is streamed directly from the file. Small adaptations can be done to read
      and process data from an in memory buffer.
    - Not intented to be used in any commercial product. Experimental only.
*/

class XMLDecoder
{
public:
    struct Node;
    using Tag = std::pair<std::string, std::string>;
    using NodeSPtr = std::shared_ptr<Node>;
    using NodeWPtr = std::weak_ptr<Node>;
    using NodeVec = std::vector<NodeSPtr>;
    using ErrorData = std::string;
    using AttrPair = std::pair<std::string, std::string>;
    using AttrPairVec = std::vector<AttrPair>;
    using XmlResult = std::pair<NodeVec, ErrorData>; // this should be changed to bellow
    // using XmlResult = std::pair<NodeSPtr, ErrorData>;
    using IndexBuffer = std::vector<uint32_t>;

    struct Node
    {
        NodeSPtr getTagNamed(const std::string& tagName);
        NodeVec getTagsNamed(const std::string& tagName);
        std::tuple<NodeVec, IndexBuffer> getTagsNamedIndexed(const std::string& tagName);
        NodeSPtr getTagNamedWithAttrib(const std::string& tagName, const AttrPair& attrib);
        std::optional<std::string> getAttribValue(const std::string& attribKey);

        void show(const uint32_t depth = 0);

        std::string nodeName;
        std::string innerText;
        AttrPairVec attributes;
        NodeWPtr parent;
        NodeVec children;
    };

    XMLDecoder() = default;

    static NodeSPtr
    getDirectChildWithTagAndAttribFromVec(const NodeVec& nodes, const std::string& tagName, const AttrPair& attrib);

    static std::tuple<XMLDecoder::NodeSPtr, uint32_t>
    selfGetDirectChildWithTagAndAttribFromVec(const NodeVec& nodes, const std::string& tagName, const AttrPair& attrib);

    XmlResult decodeFromStream(std::ifstream& stream);
    XmlResult decodeFromBuffer([[maybe_unused]] std::vector<uint8_t> buffer);

private:
    enum class State : uint8_t
    {
        Idle,
        AquireTagOpName,
        AquiredTagOpName,
        AquireTagClName,
        AquireAttribKey,
        AquiredAttribKey,
        AquireAttribValue,
        SkipComment,
    };

    XmlResult decode(std::ifstream& stream, const NodeSPtr pNode, const State startState = State::Idle);

    std::string getStateString(const State& state);

    void changeState(State& state, State newState);
};
} // namespace hk