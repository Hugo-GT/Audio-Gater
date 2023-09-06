#include "../AudioGaterApp.h"

//==============================================================================
TransportSlider::TransportSlider(juce::AudioTransportSource& audioSource) :
	audioSource(audioSource)
{
	setOpaque(true);
	startTimer(100);
}

TransportSlider::~TransportSlider()
{
}

void TransportSlider::paint(juce::Graphics& g)
{
	float playbackPos = audioSource.getCurrentPosition();
	float playbackDuration = audioSource.getLengthInSeconds();
	float maxWidth = getWidth();
	float drawPos = playbackPos / playbackDuration * maxWidth;

	g.fillAll(juce::Colours::black);
	g.setColour(juce::Colours::chartreuse);
	g.drawLine(drawPos, 0, drawPos, getHeight(), 4);
}

void TransportSlider::timerCallback()
{
	repaint();
}

void TransportSlider::mouseDown(const juce::MouseEvent& event)
{
	setNewPlaybackPosition();
}

void TransportSlider::mouseDrag(const juce::MouseEvent& event)
{
	setNewPlaybackPosition();
}

void TransportSlider::setNewPlaybackPosition()
{
	double xPos = getMouseXYRelative().getX();
	double maxWidth = getWidth();

	if (xPos > maxWidth)
		xPos = maxWidth;

	double playbackPos = xPos / maxWidth * audioSource.getLengthInSeconds();

	if (playbackPos < 0)
		playbackPos = 0;

	audioSource.setPosition(playbackPos);
}