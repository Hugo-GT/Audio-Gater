#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : state(Stopped)
{
	// Make sure you set the size of the component after
	// you add any child components.
	addAndMakeVisible(&openBtn);
	openBtn.setButtonText("Open...");
	openBtn.onClick = [this]() { openBtnClicked(); };
	openBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::blueviolet);

	addAndMakeVisible(&playBtn);
	playBtn.setButtonText("Play");
	playBtn.onClick = [this]() { playBtnClicked(); };
	playBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
	playBtn.setEnabled(false);

	addAndMakeVisible(&stopBtn);
	stopBtn.setButtonText("Stop");
	stopBtn.onClick = [this]() { stopBtnClicked(); };
	stopBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
	stopBtn.setEnabled(false);

	setSize(300, 200);

	formatManager.registerBasicFormats();       // [1]
	transportSource.addChangeListener(this);   // [2]

	setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	auto* device = deviceManager.getCurrentAudioDevice();
	auto activeInputChannels = device->getActiveInputChannels();
	auto activeOutputChannels = device->getActiveOutputChannels();
	auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
	auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

	for (auto channel = 0; channel < maxOutputChannels; ++channel)
	{
		if ((!activeOutputChannels[channel]) || maxInputChannels == 0)
		{
			bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
		}
		else
		{
			auto actualInputChannel = channel % maxInputChannels; // [1]

			if (!activeInputChannels[channel]) // [2]
			{
				bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
			}
			else // [3]
			{
				auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel,
					bufferToFill.startSample);
				auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

				for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
				{
					if (sampleCounter > 0)
						sampleCounter--;

					outBuffer[sample] = inBuffer[sample] * 10.0f;

					float test = outBuffer[sample];

					if (outBuffer[sample] >= 0.2f || outBuffer[sample] <= -0.2f)
					{
						transportSource.setGain(0.0f);
						sampleCounter = 30000;
					}
				}
			}
		}
	}


	if (sampleCounter == 0)
		transportSource.setGain(1.0f);

	if (readerSource.get() == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}
	transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
	// This will be called when the audio device stops, or when it is being
	// restarted due to a setting change.
	// For more details, see the help for AudioProcessor::releaseResources()

	transportSource.releaseResources();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

	// You can add your drawing code here!
}

void MainComponent::resized()
{
	// This is called when the MainContentComponent is resized.
	// If you add any child components, this is where you should
	// update their positions.

	int vSpacing = getHeight() * 0.04;
	int hSpacing = getWidth() * 0.04;

	juce::Grid grid;

	using Track = juce::Grid::TrackInfo;
	using Fr = juce::Grid::Fr;

	grid.templateRows = {
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1))
	};
	grid.templateColumns = {
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1))
	};

	grid.columnGap.pixels = hSpacing;
	grid.rowGap.pixels = vSpacing;

	grid.items = {
		juce::GridItem(openBtn).withArea(3,2),
		juce::GridItem(playBtn).withArea(4,2),
		juce::GridItem(stopBtn).withArea(5,2),
	};

	grid.performLayout(getLocalBounds().reduced(vSpacing));
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		if (transportSource.isPlaying())
			changeState(Playing);
		else
			changeState(Stopped);
	}
}

double MainComponent::getSongLengthInSeconds()
{
	return readerSource != nullptr ? readerSource->getAudioFormatReader()->lengthInSamples / readerSource->getAudioFormatReader()->sampleRate : 0.0;
}

double MainComponent::getCurrentPositionInSeconds()
{
	return transportSource.isPlaying() ? transportSource.getCurrentPosition() : 0.0;
}

juce::String MainComponent::formatTime(int seconds)
{
	int minutes = seconds / 60;
	int remainingSeconds = seconds % 60;

	juce::String formattedTime;
	formattedTime << juce::String(minutes).paddedLeft('0', 2) << ":" << juce::String(remainingSeconds).paddedLeft('0', 2);

	return formattedTime;
}

void MainComponent::openBtnClicked()
{
	chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
		juce::File{},
		"*.wav, *.mp3");                     // [7]

	auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     // [8]
		{
			juce::File file = fc.getResult();

			if (file != juce::File{})                                                // [9]
			{
				auto* reader = formatManager.createReaderFor(file);

				if (reader != nullptr)
				{
					auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
					transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
					playBtn.setEnabled(true);                                                      // [13]
					readerSource.reset(newSource.release());                                          // [14]
				}
			}
		});
}

void MainComponent::playBtnClicked()
{
	changeState(Starting);
}

void MainComponent::stopBtnClicked()
{
	changeState(Stopping);
}

void MainComponent::changeState(TransportState newState)
{
	if (state != newState)
	{
		state = newState;

		switch (state)
		{
		case Stopped:                           // [3]
			stopBtn.setEnabled(false);
			playBtn.setEnabled(true);
			transportSource.setPosition(0.0);
			break;

		case Starting:                          // [4]
			playBtn.setEnabled(false);
			transportSource.start();
			break;

		case Playing:                           // [5]
			stopBtn.setEnabled(true);
			break;

		case Stopping:                          // [6]
			transportSource.stop();
			break;
		}
	}
}
