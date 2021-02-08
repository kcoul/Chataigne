/*
  ==============================================================================

    LinkableParameterEditor.cpp
    Created: 21 Dec 2020 11:12:32pm
    Author:  bkupe

  ==============================================================================
*/

#include "LinkableParameterEditor.h"
#include "UI/ChataigneAssetManager.h"

LinkableParameterEditor::LinkableParameterEditor(ParameterLink* pLink, bool showMappingOptions) :
    InspectableEditor(pLink->parameter.get(), false),
    link(pLink),
    showMappingOptions(showMappingOptions)
{
    link->addAsyncParameterLinkListener(this);

    linkBT.reset(AssetManager::getInstance()->getToggleBTImage(ChataigneAssetManager::getInstance()->linkOnImage));
    linkBT->addListener(this);
    //linkBT->setToggleState(link->linkType != link->NONE, dontSendNotification);
    addAndMakeVisible(linkBT.get());

    paramEditor.reset((ParameterEditor *)pLink->parameter->getEditor(false));
    addAndMakeVisible(paramEditor.get());

    setSize(100, paramEditor->getHeight());
}

LinkableParameterEditor::~LinkableParameterEditor()
{
    if (!inspectable.wasObjectDeleted()) link->removeAsyncParameterLinkListener(this);
}

void LinkableParameterEditor::paint(Graphics& g)
{
    Colour c = NORMAL_COLOR;
    switch (link->linkType)
    {
    case ParameterLink::NONE:
        break;

    case ParameterLink::MAPPING_INPUT:
        c = BLUE_COLOR.withBrightness(.7f);
        break;

    case ParameterLink::MULTIPLEX_LIST:
        c = GREEN_COLOR.withBrightness(.7f);
        break;

    case ParameterLink::INDEX:
    case ParameterLink::INDEX_ZERO:
        c = YELLOW_COLOR.withBrightness(.7f);
        break;
    }

    g.setColour(c);
    g.fillEllipse(btRect.toFloat());
}

void LinkableParameterEditor::resized()
{
    Rectangle<int> r = getLocalBounds();
    int ts = jmin(r.getHeight(), 20);
    btRect = r.removeFromRight(ts).withHeight(ts).reduced(2);
    linkBT->setBounds(btRect);
    //linkBT->setToggleState(link->linkType != link->NONE, dontSendNotification);
    paramEditor->setBounds(r);
}

void LinkableParameterEditor::buttonClicked(Button* b)
{
    if (b == linkBT.get())
    {
        PopupMenu p;
       
        if (showMappingOptions)
        {
            PopupMenu mappingMenu;
            bool ticked = false;

            for (int i = 0; i < link->inputValueNames.size(); i++)
            {
                bool t = link->linkType == link->MAPPING_INPUT && link->mappingValueIndex == i;
                ticked |= t;
                mappingMenu.addItem(1 + i, "Value " + String(i + 1)+" : "+link->inputValueNames[i], true, t);
            }

            p.addSubMenu("From Mapping Input", mappingMenu, true, Image(), ticked);
        }

        if (link->isMultiplexed())
        {

            PopupMenu itMenu;
            
            bool ticked = link->linkType == link->INDEX_ZERO || link->linkType == link->INDEX;

            itMenu.addItem(-2, "Index (0-" + String(link->getMultiplexCount() - 1)+")", true, link->linkType == link->INDEX_ZERO);
            itMenu.addItem(-3, "Index (1-" + String(link->getMultiplexCount()) + ")", true, link->linkType == link->INDEX);
            itMenu.addSeparator();

            for (int i = 0; i < link->multiplex->listManager.items.size(); i++)
            {
                BaseMultiplexList* bli = link->multiplex->listManager.items[i];
                bool t = link->linkType == link->MULTIPLEX_LIST && link->list == bli;
                ticked |= t;

                itMenu.addItem(1000 + i, "List : " + bli->niceName, true, t);
            }

            p.addSubMenu("From Multiplex", itMenu, true, Image(), ticked);
        }

        p.addSeparator();
        p.addItem(-1, "Unlink", link->linkType != link->NONE);

        
        if (int result = p.show())
        {
            if (result == -1) link->setLinkType(link->NONE);
            else if (result == -2) link->setLinkType(link->INDEX_ZERO);
            else if (result == -3) link->setLinkType(link->INDEX);
            else if (result >= 1000)
            {
                link->setLinkType(link->MULTIPLEX_LIST);
                link->list = link->multiplex->listManager.items[result - 1000];
            }
            else
            {
                link->mappingValueIndex = result - 1;
                link->setLinkType(link->MAPPING_INPUT);
            }
        }

    }
}

void LinkableParameterEditor::childBoundsChanged(Component* c)
{
    if (c == paramEditor.get())
    {
        setSize(getWidth(), paramEditor->getHeight());
        repaint();
    }
}

void LinkableParameterEditor::newMessage(const ParameterLink::ParameterLinkEvent& e)
{
    //linkBT->setToggleState(link->linkType != link->NONE, dontSendNotification);
    repaint();
}