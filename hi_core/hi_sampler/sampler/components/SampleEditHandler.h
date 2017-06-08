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

#ifndef SAMPLEEDITHANDLER_H_INCLUDED
#define SAMPLEEDITHANDLER_H_INCLUDED

typedef Array<ModulatorSamplerSound*> SampleSelection;

class SampleEditHandler: public ChangeListener
{
public:

	struct Listener
	{
		virtual ~Listener()
		{
			masterReference.clear();
		}

		virtual void soundSelectionChanged(SampleSelection& newSelection) = 0;

	private:

		friend class WeakReference<Listener>;
		WeakReference<Listener>::Master masterReference;
	};

	SampleEditHandler(ModulatorSampler* sampler_):
		sampler(sampler_)
	{
		selectedSamplerSounds.addChangeListener(this);
	}

	~SampleEditHandler()
	{
		selectedSamplerSounds.removeChangeListener(this);
	}

	void changeListenerCallback(ChangeBroadcaster* /*source*/)
	{
		sendSelectionChangeMessage();
	}

	void addSelectionListener(Listener* l)
	{
		selectionListeners.addIfNotAlreadyThere(l);
	}

	void removeSelectionListener(Listener* l)
	{
		selectionListeners.removeAllInstancesOf(l);
	}
	
	SelectedItemSet<WeakReference<ModulatorSamplerSound>> &getSelection()
	{
		return selectedSamplerSounds;
	}

	ModulatorSampler* getSampler() { return sampler; }

	void moveSamples(SamplerSoundMap::Neighbour direction);

	void setDisplayOnlyRRGroup(int newRRIndex) { rrIndex = newRRIndex; }

	int getCurrentlyDisplayedRRGroup() const { return rrIndex; }

	void sendSelectionChangeMessage()
	{
		const uint32 thisTime = Time::getMillisecondCounter();

		if ((thisTime - timeSinceLastSelectionChange) < 1)
		{
			timeSinceLastSelectionChange = thisTime;
			return;
		}

		timeSinceLastSelectionChange = thisTime;

		const Array<WeakReference<ModulatorSamplerSound>> newSelection = selectedSamplerSounds.getItemArray();

		auto existingSounds = getSanitizedSelection();

		
		if (existingSounds != lastSelection)
		{
			for (int i = 0; i < selectionListeners.size(); i++)
			{
				selectionListeners[i]->soundSelectionChanged(existingSounds);
			}

			lastSelection.swapWith(existingSounds);
		}
	}

	void handleMidiSelection();

	struct SampleEditingActions
	{
		static void deleteSelectedSounds(SampleEditHandler *body);
		static void duplicateSelectedSounds(SampleEditHandler *body);
		static void removeDuplicateSounds(SampleEditHandler *body);
		static void cutSelectedSounds(SampleEditHandler *body);
		static void copySelectedSounds(SampleEditHandler *body);
		static void automapVelocity(SampleEditHandler *body);
		static void pasteSelectedSounds(SampleEditHandler *body);

		static void checkMicPositionAmountBeforePasting(const ValueTree &v, ModulatorSampler * s);

		static void refreshCrossfades(SampleEditHandler * body);
		static void selectAllSamples(SampleEditHandler * body);
		static void mergeIntoMultiSamples(SampleEditHandler * body, Component* childOfRoot);
		static void extractToSingleMicSamples(SampleEditHandler * body);
		static void normalizeSamples(SampleEditHandler *handler, Component* childOfRoot);
		static void automapUsingMetadata(ModulatorSampler* sampler);
		static void trimSampleStart(SampleEditHandler * body);
	};

	

private:

	SampleSelection getSanitizedSelection();

	bool newKeysPressed(const uint8 *currentNotes);

	void changeProperty(ModulatorSamplerSound *s, ModulatorSamplerSound::Property p, int delta);;

	int rrIndex = -1;

	SelectedItemSet<WeakReference<ModulatorSamplerSound>> selectedSamplerSounds;
	
	int timeSinceLastSelectionChange = 0;

	SampleSelection lastSelection;

	Array<WeakReference<Listener>> selectionListeners;

	ModulatorSampler* sampler;
};



#endif  // SAMPLEEDITHANDLER_H_INCLUDED
