#include "HkXml.hpp"

#include <cstdint>
#include <memory>
#include <ranges>

#define MUTE_PRINT
#include "Utility.hpp"

namespace hk
{

XMLDecoder::XmlResult XMLDecoder::decodeFromStream(std::ifstream& stream)
{
    return decode(stream, nullptr);
}

XMLDecoder::XmlResult decodeFromBuffer([[maybe_unused]] std::vector<uint8_t> buffer)
{
    return XMLDecoder::XmlResult{};
}

XMLDecoder::XmlResult XMLDecoder::decode(std::ifstream& stream, const NodeSPtr pNode, const State startState)
{
    std::string charAccumulator;
    State state{State::Idle};
    State stateBeforeComment{state};

    changeState(state, startState);

    NodeVec nodes;
    NodeSPtr node = std::make_shared<Node>();

    while (stream.peek() != EOF)
    {
        char ch = utils::read1(stream);

        /* We just skip over eveything unless we hit a - , this - could break us out. */
        if (state == State::SkipComment && ch != '-')
        {
            continue;
        }

        switch (ch)
        {
            case '\n': {
                /* If we see a new line while we AquireTagOpName means we have attributes inside. It's invalid
                 * for tag name to be on two lines. */
                if (state == State::AquireTagOpName && stream.peek() != '/' && stream.peek() != ' ')
                {
                    changeState(state, State::AquireAttribKey);
                }
            }
            break;
            case ' ': {
                /* If we see a space while we AquireTagOpName AND the following is not again space or new line or
                 * / or <, means we have attributes inside */
                if (state == State::AquireTagOpName && stream.peek() != ' ' && stream.peek() != '\n' &&
                    stream.peek() != '/' && stream.peek() != '>')
                {
                    changeState(state, State::AquireAttribKey);
                }
                /* If while aquiring attrib's value we see a space then we need to add it too. */
                else if (state == State::AquireAttribValue)
                {
                    charAccumulator += ch;
                }
                /* After AquiredTagOpName, we can cummulate chars between > and < as inner object text. Spaces
                 * included.*/
                else if (state == State::AquiredTagOpName && !charAccumulator.empty())
                {
                    charAccumulator += ch;
                }
                break;
            }
            case '<': {
                if (state == State::AquiredTagOpName)
                {
                    /* We have a comment as a "child" */
                    if (stream.peek() == '!')
                    {
                        utils::read2(stream);       // advance 2 chars to get over the dashes (--)
                        stateBeforeComment = state; // we may return to fetching next tag open/close, memorize which
                        changeState(state, State::SkipComment);
                    }
                    /* We either we have children */
                    else if (stream.peek() != '/')
                    {
                        /* If there was text between > and < but we still have children, this is invalid. Error
                         * out.*/
                        if (!charAccumulator.empty())
                        {
                            std::string errorString;
                            sprint(errorString,
                                "After aquire opening tag there is inner text (%s) and also more children on same "
                                "level. Invalid",
                                charAccumulator.c_str());
                            return {nodes, errorString};
                        }

                        XmlResult result = decode(stream, node, State::AquireTagOpName);

                        /* If we encountered errors, bail out */
                        if (!result.second.empty())
                        {
                            return {nodes, result.second};
                        }

                        node->children = result.first;
                    }
                    /* Or object itself will close with </ (not self close) */
                    else
                    {
                        /* Save inner text if any was accumulated */
                        node->innerText = charAccumulator;
                        charAccumulator.clear();
                        changeState(state, State::AquireTagClName);
                    }
                }
                /* Comment will follow if we have <! */
                else if (stream.peek() == '!')
                {
                    utils::read2(stream);       // advance 2 chars to get over the dashes (--)
                    stateBeforeComment = state; // we may return to fetching next tag open/close, memorize which
                    changeState(state, State::SkipComment);
                }
                /* Otherwise maybe < is part of some attrib value phrase. */
                else if (state == State::AquireAttribValue)
                {
                    charAccumulator += ch;
                }
                /* Aquire opening name only if we dont have one already */
                else if (node->nodeName.empty())
                {
                    /* This means we collected chars while on Idle aka there's node with children finishing with
                     * some inner text. Invalid.*/
                    if (!charAccumulator.empty())
                    {
                        std::string errorString;
                        sprint(errorString,
                            "After ending child there is inner text (%s) and also more children on same level. "
                            "Invalid",
                            charAccumulator.c_str());
                        return {nodes, errorString};
                    }
                    changeState(state, State::AquireTagOpName);
                }
                break;
            }
            case '>': {
                if (state == State::AquireTagOpName)
                {
                    changeState(state, State::AquiredTagOpName);
                }
                else if (state == State::AquireTagClName)
                {
                    if (charAccumulator == node->nodeName)
                    {
                        changeState(state, State::Idle);

                        /* Push node to children list */
                        nodes.emplace_back(node);
                        nodes.back()->parent = pNode;

                        /* Clear closingNameHolder, no need anymore */
                        charAccumulator.clear();

                        /* Create new node to store potential new data. Previous data is saved in node list. */
                        node = std::make_shared<Node>();
                    }
                    else
                    {
                        /* If we hit > but closing name and stored node name do not match. Error. */
                        std::string errorString;
                        sprint(errorString, "Closing tag names do not match |%s| |%s|", charAccumulator.c_str(),
                            node->nodeName.c_str());
                        return {nodes, errorString};
                    }
                }
                /* Handle cases in which we get to > while we still haven't aquired the attrib's value.*/
                else if (state == State::AquireAttribKey || state == State::AquiredAttribKey)
                {
                    std::string errorString;
                    std::string attribKey = charAccumulator.empty() ? node->attributes.back().first : charAccumulator;
                    sprint(errorString, "Reached end of opening statement but didn't find value for attrib key: |%s|",
                        attribKey.c_str());
                    return {nodes, errorString};
                }
                /* Otherwise maybe > is part of some attrib value phrase. */
                else if (state == State::AquireAttribValue)
                {
                    charAccumulator += ch;
                }
                break;
            }
            case '/': {
                /* Hitting / while AquireTagOpName means we have self closing object with no inner attributes */
                if (state == State::AquireTagOpName)
                {
                    /* Check if tag is self closing (same lvl only) */
                    if (stream.peek() == '>')
                    {
                        // nodes.push_back(node);
                        nodes.emplace_back(node);
                        nodes.back()->parent = pNode;
                        changeState(state, State::Idle);

                        /* Clear closingNameHolder, no need anymore */
                        charAccumulator.clear();

                        /* Create new node to store potential new data. Previous data is saved in node list. */
                        node = std::make_shared<Node>();
                    }
                    /* Else it might be the case that this level is done and we should bubble up to where we came
                     * from and continue there.*/
                    else
                    {
                        stream.seekg(-2, std::ios::cur);
                        return {nodes, ""};
                    }
                }
                /* Otherwise maybe / is part of some attrib value phrase. */
                else if (state == State::AquireAttribValue)
                {
                    charAccumulator += ch;
                }
                break;
            }
            case '=': {
                /* If we hit = and we are in AquireAttribKey state means we finished reading the attrib's key and
                 * what follows is it's value. So store the key and reset the accumulator.*/
                if (state == State::AquireAttribKey)
                {
                    changeState(state, State::AquiredAttribKey);
                    node->attributes.emplace_back(charAccumulator, "");
                    charAccumulator.clear();
                }
                break;
            }
            case '"': {
                /* If we have already aquired the attrib key and we see " means the key's value will follow, so
                 * change state.*/
                if (state == State::AquiredAttribKey)
                {
                    changeState(state, State::AquireAttribValue);
                }
                /* Seeing " again while aquiring the attrib value means the value ends now. Next go back to aquiring
                 * the tag name."*/
                else if (state == State::AquireAttribValue)
                {
                    node->attributes.back().second = charAccumulator;
                    charAccumulator.clear();
                    changeState(state, State::AquireTagOpName);
                }
                break;
            }
            case '?': {
                /* If we get in this case, means the XML header will close with ?> . Do something similar as in /
                 * case.*/
                if (state == State::AquireTagOpName && stream.peek() == '>')
                {
                    // nodes.push_back(node);
                    nodes.emplace_back(node);
                    nodes.back()->parent = pNode;
                    changeState(state, State::Idle);

                    /* Clear closingNameHolder, no need anymore */
                    charAccumulator.clear();

                    /* Create new node to store potential new data. Previous data is saved in node list. */
                    node = std::make_shared<Node>();
                    break;
                }

                /* Otherwise there is justified fall through as ? might be part of some text. */
                [[fallthrough]];
            }
            case '-': {
                if (state == State::SkipComment && stream.peek() == '-')
                {
                    utils::read1(stream); // read the > that follows (-->)
                    changeState(state, stateBeforeComment);
                    break;
                }
                /* Otherwise there is justified fall through as - might be part of some text. */
                [[fallthrough]];
            }
            default: {
                if (state == State::AquireTagOpName)
                {
                    node->nodeName += ch;
                }

                /* The 3 ifs do the same thing in different situations. This is explicitly left like this for
                 * readability. */
                if (state == State::AquireTagClName)
                {
                    charAccumulator += ch;
                }

                /* After we aquired Op or we are Idle (moving forward to check for next Op) we can cummulate chars
                 * between > and < as inner object text. Spaces included.*/
                if (state == State::AquiredTagOpName || state == State::Idle)
                {
                    charAccumulator += ch;
                }

                /* Aquire the tag of course */
                if (state == State::AquireAttribKey || state == State::AquireAttribValue)
                {
                    charAccumulator += ch;
                }
                break;
            }
        }
    }

    /* If we reach EOF but we didnt find a closing object for out node. Error out.*/
    if (stream.peek() == EOF && !node->nodeName.empty())
    {
        std::string errorString;
        sprint(errorString, "Reached end of data but no matching closing object for tag: |%s|", node->nodeName.c_str());
        return {nodes, errorString};
    }

    return {nodes, ""};
}

void XMLDecoder::changeState(State& state, State newState)
{
    if (state == newState)
    {
        println("State is already %s", getStateString(newState).c_str());
        return;
    }
    println("Switching from %s to %s", getStateString(state).c_str(), getStateString(newState).c_str());
    state = newState;
}

std::string XMLDecoder::getStateString(const State& state)
{
#define STATE_CASE(x)                                                                                                  \
    case x:                                                                                                            \
        return "<" #x ">"

    switch (state)
    {
        STATE_CASE(State::Idle);
        STATE_CASE(State::AquireTagOpName);
        STATE_CASE(State::AquiredTagOpName);
        STATE_CASE(State::AquireTagClName);
        STATE_CASE(State::AquireAttribKey);
        STATE_CASE(State::AquiredAttribKey);
        STATE_CASE(State::AquireAttribValue);
        STATE_CASE(State::SkipComment);
    }
#undef STATE_CASE
    return "<state unknown>";
}

XMLDecoder::NodeSPtr XMLDecoder::Node::getTagNamed(const std::string& tagName)
{
    NodeVec nv = getTagsNamed(tagName);
    return nv.empty() ? nullptr : nv[0];
}

XMLDecoder::NodeVec XMLDecoder::Node::getTagsNamed(const std::string& tagName)
{
    const auto tagNamedPred = [tagName](XMLDecoder::NodeSPtr node) { return node->nodeName == tagName; };
    return children | std::views::filter(tagNamedPred) | std::ranges::to<std::vector>();
}

std::tuple<XMLDecoder::NodeVec, XMLDecoder::IndexBuffer>
XMLDecoder::Node::getTagsNamedIndexed(const std::string& tagName)
{
    const auto tagNamedPred = [tagName](XMLDecoder::NodeSPtr node) { return node->nodeName == tagName; };

    NodeVec nv;
    IndexBuffer indexBuffer;
    int32_t index{0};
    for (const auto& ch : children)
    {
        if (ch->nodeName == tagName)
        {
            indexBuffer.push_back(index);
            nv.push_back(ch);
        }
        index++;
    }
    return {nv, indexBuffer};
}

XMLDecoder::NodeSPtr XMLDecoder::Node::getTagNamedWithAttrib(const std::string& tagName, const AttrPair& searchAttrib)
{
    const auto attribPred = [searchAttrib](XMLDecoder::AttrPair attr)
    { return attr.first == searchAttrib.first && attr.second == searchAttrib.second; };

    const XMLDecoder::NodeVec& nodes = getTagsNamed(tagName);
    for (const auto& node : nodes)
    {
        auto result = node->attributes | std::views::filter(attribPred);
        if (!std::ranges::empty(result))
        {
            return node;
        }
    }
    return nullptr;
}

std::optional<std::string> XMLDecoder::Node::getAttribValue(const std::string& attribKey)
{
    for (const auto& attrib : attributes)
    {
        if (attrib.first == attribKey)
        {
            return attrib.second;
        }
    }
    return {};

    // const auto attribKeyPred = [attribKey](XMLDecoder::AttrPair attrPair) { return attrPair.first == attribKey; };
    // XMLDecoder::AttrPairVec attrib = attributes | std::views::filter(attribKeyPred) | std::ranges::to<std::vector>();
    // if (attrib.empty())
    // {
    //     return {};
    // }
    // return attrib[0].second;
}

XMLDecoder::NodeSPtr XMLDecoder::getDirectChildWithTagAndAttribFromVec(const NodeVec& nodes,
    const std::string& tagName,
    const AttrPair& searchAttrib)
{
    auto attribPred = [searchAttrib](const XMLDecoder::AttrPair attr)
    { return attr.first == searchAttrib.first && attr.second == searchAttrib.second; };

    for (const auto& node : nodes)
    {
        XMLDecoder::NodeSPtr protoNode = node->getTagNamedWithAttrib(tagName, searchAttrib);
        if (protoNode)
        {
            return protoNode;
        }
    }
    return nullptr;
}

std::tuple<XMLDecoder::NodeSPtr, uint32_t> XMLDecoder::selfGetDirectChildWithTagAndAttribFromVec(const NodeVec& nodes,
    const std::string& tagName,
    const AttrPair& searchAttrib)
{
    auto attribPred = [searchAttrib](const XMLDecoder::AttrPair attr)
    { return attr.first == searchAttrib.first && attr.second == searchAttrib.second; };

    int32_t index{0};
    for (const auto& node : nodes)
    {
        XMLDecoder::NodeSPtr protoNode = node->getTagNamedWithAttrib(tagName, searchAttrib);
        if (protoNode)
        {
            return {node, index};
        }
        index++;
    }
    return {nullptr, 0};
}

void XMLDecoder::Node::show(const uint32_t depth)
{
    for (uint32_t i = 0; i < depth; ++i)
    {
        print("    ");
    }
    print("|---");
    print("<object> %s ", nodeName.c_str());

    if (!innerText.empty())
    {
        print("(%s) ", innerText.c_str());
    }

    for (const auto& [key, val] : attributes)
    {
        print("[%s, %s]", key.c_str(), val.c_str());
    }

    std::string pName{"None"};
    if (const auto& p = parent.lock())
    {
        pName = p->nodeName;
    }
    print("{%s}\n", pName.c_str());

    for (const auto& node : children)
        node->show(depth + 1);
}
} // namespace hk