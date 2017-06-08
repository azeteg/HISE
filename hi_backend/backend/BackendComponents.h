/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for cloused source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef BACKENDCOMPONENTS_H_INCLUDED
#define BACKENDCOMPONENTS_H_INCLUDED


class BackendProcessorEditor;
class ScriptContentContainer;


class MacroParameterTable;

/** A component which shows eight knobs that control eight macro controls and allows editing of the mapped parameters.
*	@ingroup macroControl
*/
class MacroComponent: public Component,
					  public ButtonListener,
					  public SafeChangeListener,
					  public SliderListener,
					  public LabelListener
{
public:

	MacroComponent(BackendRootWindow* rootWindow);;

	~MacroComponent();

	SET_GENERIC_PANEL_ID("MacroControls");

	struct MacroControlPopupData
	{
		int itemId;
		ModulatorSynthChain *chain;
		int macroIndex;
	};

	void mouseDown(const MouseEvent &e);

	void addSynthChainToPopup(ModulatorSynthChain *parent, PopupMenu &p, Array<MacroControlPopupData> &popupData);
	

	void setSynthChain(ModulatorSynthChain *synthChainToControl)
	{
		synthChain = synthChainToControl;
	}

	void checkActiveButtons()
	{
		for(int i = 0; i < editButtons.size(); i++)
		{
			const bool on = synthChain->hasActiveParameters(i);

			macroNames[i]->setColour(Label::ColourIds::backgroundColourId, on ? Colours::black.withAlpha(0.1f) : Colours::transparentBlack );			
			macroNames[i]->setColour(Label::ColourIds::textColourId, on ? Colours::white : Colours::white.withAlpha(0.4f) );			
			macroNames[i]->setEnabled(on);
		}

		for(int i = 0; i < macroKnobs.size(); i++)
		{
			macroKnobs[i]->setEnabled(synthChain->hasActiveParameters(i));
		}

		for(int i = 0; i < macroNames.size(); i++)
		{
			if(macroNames[i]->getText() != synthChain->getMacroControlData(i)->getMacroName())
			{
				macroNames[i]->setText(synthChain->getMacroControlData(i)->getMacroName(), dontSendNotification);
			}
		}

	}


	void changeListenerCallback(SafeChangeBroadcaster *);
	
	MacroParameterTable* getMainTable();

	int getCurrentHeight()
	{
#if HISE_IOS
		return 200;
#else
		return 90;
#endif
	}

	void resized();

	void sliderValueChanged(Slider *s)
	{
		const int macroIndex = macroKnobs.indexOf(s);

		processor->getMainSynthChain()->setMacroControl(macroIndex, (float)s->getValue(), sendNotification);

		//processor->setParameterNotifyingHost(macroIndex, (float)s->getValue() / 127.0f);
	}

	void labelTextChanged(Label *l)
	{
		for(int i = 0; i< macroNames.size(); i++)
		{
			if(macroNames[i] == l) synthChain->getMacroControlData(i)->setMacroName(l->getText());
		}
		
	}

	void buttonClicked(Button *b);

	private:

	friend class MacroParameterTable;

	ScopedPointer<MacroKnobLookAndFeel> mlaf;

	BackendRootWindow* rootWindow;

	BackendProcessor *processor;

	ModulatorSynthChain *synthChain;

	OwnedArray<Slider> macroKnobs;
	OwnedArray<Label> macroNames;
	OwnedArray<ShapeButton> editButtons;

	
};

class BreadcrumbComponent : public Component,
							public MainController::ProcessorChangeHandler::Listener
{
public:
	BreadcrumbComponent(MainController* mc_);;

	~BreadcrumbComponent();

	void moduleListChanged(Processor* /*processorThatWasChanged*/, MainController::ProcessorChangeHandler::EventType type)
	{
		if (type == MainController::ProcessorChangeHandler::EventType::ProcessorRenamed)
			refreshBreadcrumbs();
	}

	void paint(Graphics &g) override
	{
		g.setColour(Colour(0xFF383838));

		g.fillRoundedRectangle(0.0f,0.0f,(float)getWidth(), (float)getHeight()-8.0f, 3.0f);

		for (int i = 1; i < breadcrumbs.size(); i++)
		{
			g.setColour(Colours::white.withAlpha(0.6f));
			g.setFont(GLOBAL_BOLD_FONT());
			g.drawText(">" , breadcrumbs[i]->getRight(), breadcrumbs[i]->getY(), 20, breadcrumbs[i]->getHeight(), Justification::centred, true);
		}
		
	}

	void refreshBreadcrumbs();

    void resized();
    
private:

	class Breadcrumb : public Component
	{
	public:
        
        Breadcrumb(const Processor *p):
        processor(const_cast<Processor*>(p)),
		isOver(false)
        {
			setRepaintsOnMouseActivity(true);
		};

        int getPreferredWidth() const
        {
            if(processor.get() != nullptr)
            {
#if HISE_IOS
				Font f = GLOBAL_BOLD_FONT().withHeight(18.0f);
#else
                Font f = GLOBAL_BOLD_FONT();
#endif
                
                return f.getStringWidth(processor->getId());
            }
			return 0;
        }
        
		void paint(Graphics &g) override
		{
            if(processor.get() != nullptr)
            {
				g.setColour(Colours::white.withAlpha(isMouseOver(true) ? 1.0f : 0.6f));

#if HISE_IOS
				Font f = GLOBAL_BOLD_FONT().withHeight(18.0f);
#else
				Font f = GLOBAL_BOLD_FONT();
#endif

                g.setFont(f);
                g.drawText(processor->getId(), getLocalBounds(), Justification::centredLeft, true);
            }
			
		}
        
		void mouseDown(const MouseEvent& /*event*/) override;

    private:
        
        const WeakReference<Processor> processor;
		bool isOver;
	};

	OwnedArray<Breadcrumb> breadcrumbs;

	MainController* mc;
};

class BaseDebugArea;

/** A table that contains every mapped parameter for the currently edited macro slot.
*	@ingroup debugComponents
*
*	You can change the parameter range and invert it.
*/
class MacroParameterTable      :	public Component,
									public TableListBoxModel
{
public:

	enum ColumnId
	{
		ProcessorId = 1,
		ParameterName,
		Inverted,
		Minimum,
		Maximum,
		numColumns
	};

	MacroParameterTable(BackendRootWindow* /*rootWindow*/)   :
		font (GLOBAL_FONT()),
		data(nullptr)
	{
		setName("Macro Control Parameter List");

		// Create our table component and add it to this component..
		addAndMakeVisible (table);
		table.setModel (this);

		// give it a border

		table.setColour (ListBox::outlineColourId, Colours::black.withAlpha(0.5f));
		table.setColour(ListBox::backgroundColourId, HiseColourScheme::getColour(HiseColourScheme::ColourIds::DebugAreaBackgroundColourId));

		table.setOutlineThickness (0);

		laf = new TableHeaderLookAndFeel();

		table.getHeader().setLookAndFeel(laf);
		table.getHeader().setSize(getWidth(), 22);

		table.getViewport()->setScrollBarsShown(true, false, true, false);

		table.getHeader().setInterceptsMouseClicks(false, false);

		table.getHeader().addColumn("Processor", ProcessorId, 90);
		table.getHeader().addColumn("Parameter", ParameterName, 90);
		table.getHeader().addColumn("Inverted", Inverted, 50);
		table.getHeader().addColumn("Min", Minimum, 70);
		table.getHeader().addColumn("Max", Maximum, 70);

		setWantsKeyboardFocus(true);
	}

	SET_GENERIC_PANEL_ID("MacroTable");

	int getNumRows() override
	{
		return data == nullptr ? 0 : data->getNumParameters();
	};

	void updateContent()
	{
		if(data != nullptr) data->clearDanglingProcessors();

		table.updateContent();
	}

	void setMacroController(ModulatorSynthChain::MacroControlData *newData)
	{
		data = newData;

		setName("Macro Control Parameter List: " + (newData != nullptr ? newData->getMacroName() : "Idle"));

		updateContent();

		if(getParentComponent() != nullptr) getParentComponent()->repaint();
	}

	void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
	{
			
			

		if(rowNumber % 2)
		{
			g.setColour(Colours::black.withAlpha(0.05f));

			g.fillAll();
		}

		if (rowIsSelected)
		{
			
			g.fillAll (Colour(0x44000000));
			

		}
	}

	bool keyPressed(const KeyPress& key) override
	{
		if (key == KeyPress::deleteKey)
		{
			if (data != nullptr)
			{
				data->removeParameter(table.getSelectedRow());
				table.updateContent();
				return true;
			}
		}

		return false;
	}

	void setRangeValue(int row, ColumnId column, double newRangeValue)
	{
		jassert(data != nullptr);

		if(data != nullptr)
		{
			if(column == Minimum) data->getParameter(row)->setRangeStart(newRangeValue);
			else if(column == Maximum) data->getParameter(row)->setRangeEnd(newRangeValue);
		}
	};

	void setInverted(int row, bool value)
	{
		jassert(data != nullptr);

		if(data != nullptr)
		{
			data->getParameter(row)->setInverted(value);
				
		}

	}

	void selectedRowsChanged(int /*lastRowSelected*/) {};

	Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                    Component* existingComponentToUpdate) override
	{
		

		if (columnId == Minimum || columnId == Maximum)
		{
			ValueSliderColumn* slider = dynamic_cast<ValueSliderColumn*> (existingComponentToUpdate);

				
			if (slider == nullptr)
				slider = new ValueSliderColumn(*this);

			ModulatorSynthChain::MacroControlledParameterData *pData = data->getParameter(rowNumber);

			const double value = pData->getParameterRangeLimit(columnId == Maximum);

			slider->setRowAndColumn (rowNumber, (ColumnId)columnId, value, pData->getTotalRange());

			return slider;
		}
		else if(columnId == Inverted)
		{
			InvertedButton* b = dynamic_cast<InvertedButton*> (existingComponentToUpdate);

				
			if (b == nullptr)
				b = new InvertedButton(*this);

			ModulatorSynthChain::MacroControlledParameterData *pData = data->getParameter(rowNumber);

			const bool value = pData->isInverted();

			b->setRowAndColumn(rowNumber, value);

			return b;
		}
		{
			// for any other column, just return 0, as we'll be painting these columns directly.

			jassert (existingComponentToUpdate == nullptr);
			return nullptr;
		}
	}

	void paintCell (Graphics& g, int rowNumber, int columnId,
					int width, int height, bool /*rowIsSelected*/) override
	{
		g.setColour (Colours::white.withAlpha(0.8f));
		g.setFont (font);

		String text;

		if (data->getParameter(rowNumber) == nullptr)
			return;

		if(data->getParameter(rowNumber)->getProcessor() == nullptr)
		{
			return;
		}

		switch(columnId)
		{
		case ProcessorId:	text << data->getParameter(rowNumber)->getProcessor()->getId(); break;
		case ParameterName: text << data->getParameter(rowNumber)->getParameterName(); break;
			

		}

		g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);

	}


	//==============================================================================
	void resized() override
	{
		table.setBounds(getLocalBounds());

			
	}


private:

	class ValueSliderColumn: public Component,
						public SliderListener
	{
	public:
		ValueSliderColumn(MacroParameterTable &table):
			owner(table)
		{
			addAndMakeVisible(slider = new Slider());

			slider->setLookAndFeel(&laf);
			slider->setSliderStyle (Slider::LinearBar);
			slider->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
			slider->setColour (Slider::backgroundColourId, Colour (0x38ffffff));
			slider->setColour (Slider::thumbColourId, Colour (SIGNAL_COLOUR));
			slider->setColour (Slider::rotarySliderOutlineColourId, Colours::black);
			slider->setColour (Slider::textBoxOutlineColourId, Colour (0x38ffffff));
			slider->setColour (Slider::textBoxTextColourId, Colours::black);
			slider->setTextBoxIsEditable(true);

			slider->addListener(this);
		}

		void resized()
		{
			slider->setBounds(getLocalBounds());
		}

		void setRowAndColumn (const int newRow, ColumnId column, double value, NormalisableRange<double> range)
		{
			row = newRow;
			columnId = column;

			slider->setRange(range.start, range.end, 0.1);

			slider->setValue(value, dontSendNotification);
		}

	private:

		void sliderValueChanged (Slider *) override
		{
			owner.setRangeValue(row, columnId, slider->getValue());
		}

	private:
		MacroParameterTable &owner;

		HiPropertyPanelLookAndFeel laf;
				
		int row;
		ColumnId columnId;
		ScopedPointer<Slider> slider;
	};

	class InvertedButton: public Component,
							public ButtonListener
	{
	public:

		InvertedButton(MacroParameterTable &owner_):
			owner(owner_)
		{
			addAndMakeVisible(t = new TextButton("Inverted"));
			t->setButtonText("Inverted");
			t->setLookAndFeel(&laf);
			t->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
			t->addListener (this);
			t->setTooltip("Invert the range of the macro control for this parameter.");
			t->setColour (TextButton::buttonColourId, Colour (0x88000000));
			t->setColour (TextButton::buttonOnColourId, Colour (0x88FFFFFF));
			t->setColour (TextButton::textColourOnId, Colour (0xaa000000));
			t->setColour (TextButton::textColourOffId, Colour (0x99ffffff));			
			
			t->setClickingTogglesState(true);
		};

		void resized()
		{
			t->setBounds(getLocalBounds());
		}

		void setRowAndColumn (const int newRow, bool value)
		{
			row = newRow;
				
			t->setToggleState(value, dontSendNotification);

			t->setButtonText(value ? "Inverted" : "Normal");

		}

		void buttonClicked(Button *b)
		{
			t->setButtonText(b->getToggleState() ? "Inverted" : "Normal");
			owner.setInverted(row, b->getToggleState());
		};

	private:

		MacroParameterTable &owner;
				
		int row;
		ColumnId columnId;
		ScopedPointer<TextButton> t;

		HiPropertyPanelLookAndFeel laf;


	};

	TableListBox table;     // the table component itself
	Font font;

	ScopedPointer<TableHeaderLookAndFeel> laf;

	ModulatorSynthChain::MacroControlData *data;

	int numRows;            // The number of rows of data we've got

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MacroParameterTable)
};


/** A viewport that caches its content when it begins scrolling */
class CachedViewport : public Component,
	public DragAndDropTarget
{
public:

	CachedViewport();

	enum ColourIds
	{
		backgroundColourId = 0x1004
	};

	bool isInterestedInDragSource(const SourceDetails &dragSourceDetails) override;

	void itemDragEnter(const SourceDetails &dragSourceDetails) override;

	void itemDragExit(const SourceDetails &dragSourceDetails) override;

	void itemDropped(const SourceDetails &dragSourceDetails) override;;

	void paint(Graphics& g)
	{
		g.fillAll(Colour(0xFF333333));
	}

	void resized();

	
private:

	class InternalViewport : public Viewport
	{
	public:


		InternalViewport();


		bool isCurrentlyScrolling;

		Image cachedImage;

		void paint(Graphics &g);

	};



public:

	ScopedPointer<InternalViewport> viewport;

	bool dragNew;

};




#endif  // BACKENDCOMPONENTS_H_INCLUDED
