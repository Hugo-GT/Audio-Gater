#pragma once

#include <JuceHeader.h>

class TransportSlider : public juce::Component, private juce::Timer
{
public:
	TransportSlider(juce::AudioTransportSource& audioSource);
	~TransportSlider();
	void paint(juce::Graphics& g) override;
private:
	void timerCallback() override;

	juce::AudioTransportSource& audioSource;

	void mouseDown(const juce::MouseEvent& event) override;
	void mouseDrag(const juce::MouseEvent& event) override;
	void setNewPlaybackPosition();
};

class TransportBar : public juce::Component, private juce::ChangeListener
{
public:
	TransportBar(juce::AudioFormatManager& formatManager, juce::AudioTransportSource& audioSource);
	~TransportBar();
	void paint(juce::Graphics& g) override;
	void setSource(juce::File& file);
private:
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void TransportBar::paintIfNoFileLoaded(juce::Graphics& g);
	void TransportBar::paintIfFileLoaded(juce::Graphics& g);

	juce::AudioThumbnailCache audioThumbnailCache;
	juce::AudioThumbnail audioThumbnail;
	TransportSlider transportSlider;
};


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
		Starting,
		Started,
		Stopping,
		Stopped,
		Paused
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
	juce::TextButton pauseBtn;
	juce::TextButton stopBtn;
	juce::TextButton muteBtn;
	juce::TextButton unmuteBtn;

	juce::Label muteText;

	TransportBar transportBar;
	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	std::unique_ptr<juce::FileChooser> chooser;

	void openBtnClicked();
	void playBtnClicked();
	void stopBtnClicked();
	void muteBtnClicked();
	void pauseBtnClicked();
	void unmuteBtnClicked();
	void changeState(TransportState newState);

	TransportState state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioGaterApp)
};

