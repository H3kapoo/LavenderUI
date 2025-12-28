- App
    - Dropdown
        - Button
        - Dropdown
            - Button

- Dropdown (no Dropdown parent)
    - Button (has dropdown parent) -> this somehow needs to be created inside the rule
                                   by createOption() and have all the options the user put
                                   in the XML tag of the object
    - Dropdown (has dropdown parent) -> this again needs to be created via createSubmenu()
                                    with all the user options, but it also needs to be
                                    the parent of the next Button under it

Currently:

node::UIBasePtr uiParentNode = tryConstructUINode(xmlNode);

for (const auto& childXmlNode : xmlNode->children)
{
    const auto& childUiNode = tryConstructTreeFromXmlNode(childXmlNode);
    ruleMap_[xmlNode->nodeName].addRule(uiParentNode, childUiNode);
}

return uiParentNode;

Now:

// xmlNode = xmlParserGetRoot();
rootNode = AppRule["app"]->construct(nullptr, xmlNode);

app::contruct(Obj parent, xmlNode node) -> App
{
    App obj;
    // translate stuff relating to App

    for (const auto& childXmlNode : node->children)
    {
        Obj child = Rules[childXmlNode->name]->construct(obj, childXmlNode);
        obj->add(child);
    }

    return obj;
}

// parent: App, I am Dropdown
dropdown::contruct(Obj parent, xmlNode node) -> Dropdown
{
    Dropdown obj;
    if (parent.getType() != Dropdown::type)
    {
        // translate stuff relating to Dd
    }

    for (const auto& childXmlNode : node->children)
    {
        if (parent.getType() == Dropdown::type)
        {
            if (childXmlNode == "Dropdown")
            {
                auto dropdown = parent->createSubmenu(childXmlNode.tag.name);
                Rules[childXmlNode->name]->parseAndApply(dropdown, childXmlNode.tags);
                Rules[childXmlNode->name]->construct(parent, childXmlNode);
                // dont need to return anything as every needed child will be added to "parent"
                // it can return just the null dd object, it wont be used anyway
            }
            if (childXmlNode == "Button")
            {
                auto button = parent->createOption(childXmlNode.tag.name);
                Rules[childXmlNode->name]->parseAndApply(button);
            }
        }
    }

    return obj;
}
