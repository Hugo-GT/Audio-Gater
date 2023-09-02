#include "AudioGaterApp.h"

//==============================================================================
AudioGaterApp::AudioGaterApp() : state(Stopped)
{
	addAndMakeVisible(&openBtn);
	openBtn.setButtonText("Open...");
	openBtn.onClick = [this]()
	{ openBtnClicked(); };
	openBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::blueviolet);

	addAndMakeVisible(&playBtn);
	playBtn.setButtonText("Play");
	playBtn.onClick = [this]()
	{ playBtnClicked(); };
	playBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
	playBtn.setEnabled(false);

	addAndMakeVisible(&stopBtn);
	stopBtn.setButtonText("Stop");
	stopBtn.onClick = [this]()
	{ stopBtnClicked(); };
	stopBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
	stopBtn.setEnabled(false);

	addAndMakeVisible(&muteBtn);
	muteBtn.setButtonText("Mute");
	muteBtn.onClick = [this]()
	{ muteBtnClicked(); };
	muteBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::dimgrey);
	muteBtn.setEnabled(true);

	addAndMakeVisible(&unmuteBtn);
	unmuteBtn.setAlpha(0.0f);
	unmuteBtn.onClick = [this]()
	{unmuteBtnClicked(); };
	unmuteBtn.setEnabled(true);
	unmuteBtn.setVisible(false);
	unmuteBtn.setAlwaysOnTop(true);

	addAndMakeVisible(&pauseBtn);
	pauseBtn.setButtonText("Pause");
	pauseBtn.onClick = [this]()
	{pauseBtnClicked(); };
	pauseBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::slategrey);
	pauseBtn.setEnabled(false);

	addAndMakeVisible(&muteText);
	muteText.setFont(juce::Font(190.0f, juce::Font::bold));
	muteText.setColour(juce::Label::textColourId, juce::Colours::black);
	muteText.setText("AUDIO MUTED", juce::dontSendNotification);
	muteText.setJustificationType(juce::Justification::centred);
	muteText.setVisible(false);

	setSize(960, 540);

	formatManager.registerBasicFormats();
	transportSource.addChangeListener(this);

	setAudioChannels(2, 2);
}

AudioGaterApp::~AudioGaterApp()
{
	shutdownAudio();
}

//==============================================================================
void AudioGaterApp::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioGaterApp::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
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
			auto actualInputChannel = channel % maxInputChannels;

			if (!activeInputChannels[channel] || audioIsMuted)
			{
				bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
			}
			else
			{
				auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel,
					bufferToFill.startSample);
				auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

				for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
				{
					if (sampleCounter > 0)
						sampleCounter--;

					outBuffer[sample] = inBuffer[sample] * 10.0f;

					if (outBuffer[sample] >= 0.2f || outBuffer[sample] <= -0.2f)
					{
						sampleCounter = 30000;
					}
				}
			}
		}
	}

	if (state != Paused)
	{
		if (sampleCounter == 0 && !audioIsMuted)
		{
			transportSource.getNextAudioBlock(bufferToFill);
		}
		else
		{
			transportSource.setPosition(transportSource.getCurrentPosition() + (bufferToFill.numSamples / device->getCurrentSampleRate()));
		}
	}
}

void AudioGaterApp::releaseResources()
{
	transportSource.releaseResources();
}

//==============================================================================
void AudioGaterApp::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioGaterApp::resized()
{
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
		Track(Fr(1)) };
	grid.templateColumns = {
		Track(Fr(1)),
		Track(Fr(1)),
		Track(Fr(1)) };

	grid.columnGap.pixels = hSpacing;
	grid.rowGap.pixels = vSpacing;

	grid.items = {
		juce::GridItem(openBtn).withArea(2, 2),
		juce::GridItem(playBtn).withArea(3, 2),
		juce::GridItem(pauseBtn).withArea(4, 2),
		juce::GridItem(stopBtn).withArea(5, 2),
		juce::GridItem(muteBtn).withArea(6, 2)
	};

	grid.performLayout(getLocalBounds().reduced(vSpacing));

	muteText.setBounds(0, 0, getWidth(), getHeight());
	unmuteBtn.setBounds(0, 0, getWidth(), getHeight());
}

void AudioGaterApp::timerCallback()
{
	if (!muteMessageIsOn && audioIsMuted)
		muteText.setVisible(true);
	else
		muteText.setVisible(false);

	muteMessageIsOn = !muteMessageIsOn;
	startTimer(500);
}

void AudioGaterApp::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		if (transportSource.isPlaying())
			changeState(Started);
		else
			changeState(Stopped);
	}
}

double AudioGaterApp::getSongLengthInSeconds()
{
	return readerSource != nullptr ? readerSource->getAudioFormatReader()->lengthInSamples / readerSource->getAudioFormatReader()->sampleRate : 0.0;
}

double AudioGaterApp::getCurrentPositionInSeconds()
{
	return transportSource.isPlaying() ? transportSource.getCurrentPosition() : 0.0;
}

juce::String AudioGaterApp::formatTime(int seconds)
{
	int minutes = seconds / 60;
	int remainingSeconds = seconds % 60;

	juce::String formattedTime;
	formattedTime << juce::String(minutes).paddedLeft('0', 2) << ":" << juce::String(remainingSeconds).paddedLeft('0', 2);

	return formattedTime;
}

void AudioGaterApp::openBtnClicked()
{
	chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
		juce::File{},
		"*.wav, *.mp3");

	auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

	chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
		{
			juce::File file = fc.getResult();

			if (file != juce::File{})
			{
				auto* reader = formatManager.createReaderFor(file);

				if (reader != nullptr)
				{
					auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
					transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
					changeState(Stopped);
					readerSource.reset(newSource.release());
				}
			} });
}

void AudioGaterApp::playBtnClicked()
{
	changeState(Starting);
}

void AudioGaterApp::stopBtnClicked()
{
	changeState(Stopping);
}

void AudioGaterApp::muteBtnClicked()
{
	audioIsMuted = true;
	unmuteBtn.setVisible(true);
	muteText.setVisible(true);
	startTimer(500);
}

void AudioGaterApp::pauseBtnClicked()
{
	changeState(Paused);
}

void AudioGaterApp::unmuteBtnClicked()
{
	audioIsMuted = false;
	unmuteBtn.setVisible(false);
	muteText.setVisible(false);
	stopTimer();
}

void AudioGaterApp::changeState(TransportState newState)
{
	state = newState;

	switch (state)
	{
	case Stopped:
		stopBtn.setEnabled(false);
		pauseBtn.setEnabled(false);
		playBtn.setEnabled(true);
		transportSource.setPosition(0.0);
		break;

	case Stopping:
		transportSource.stop();
		break;

	case Started:
		stopBtn.setEnabled(true);
		break;

	case Starting:
		playBtn.setEnabled(false);
		pauseBtn.setEnabled(true);
		transportSource.start();
		break;

	case Paused:
		playBtn.setEnabled(true);
		pauseBtn.setEnabled(false);
		break;
	}
}
