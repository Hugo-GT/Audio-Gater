#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class AudioGaterApp : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
	//==============================================================================
	AudioGaterApp();
	~AudioGaterApp() override;

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override; // How can I see from where this function gets called (theres like 30 possibilities)
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	//==============================================================================
	// Your private member variables go here...
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	int sampleCounter = 0;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	double getSongLengthInSeconds();
	double getCurrentPositionInSeconds();
	juce::String formatTime(int seconds);

	juce::TextButton openBtn;
	juce::TextButton playBtn;
	juce::TextButton stopBtn;

	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	std::unique_ptr<juce::FileChooser> chooser;

	void openBtnClicked();
	void playBtnClicked();
	void stopBtnClicked();
	void changeState(TransportState newState);

	TransportState state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioGaterApp)
};

