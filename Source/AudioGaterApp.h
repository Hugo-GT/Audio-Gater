#pragma once

#include <JuceHeader.h>

//==============================================================================
class AudioGaterApp : public juce::AudioAppComponent, public juce::ChangeListener, private juce::Timer
{
public:
	//==============================================================================
	AudioGaterApp();
	~AudioGaterApp() override;

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	//==============================================================================
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	void timerCallback() override;

	bool muteMessageIsOn = false;
	bool audioIsMuted = false;
	int sampleCounter = 0;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	double getSongLengthInSeconds();
	double getCurrentPositionInSeconds();
	juce::String formatTime(int seconds);

	juce::TextButton openBtn;
	juce::TextButton playBtn;
	juce::TextButton stopBtn;
	juce::TextButton muteBtn;
	juce::TextButton unmuteBtn;

	juce::Label muteText;

	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	std::unique_ptr<juce::FileChooser> chooser;

	void openBtnClicked();
	void playBtnClicked();
	void stopBtnClicked();
	void muteBtnClicked();
	void unmuteBtnClicked();
	void changeState(TransportState newState);

	TransportState state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioGaterApp)
};

