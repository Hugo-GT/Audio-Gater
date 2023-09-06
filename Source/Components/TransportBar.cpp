#include "../AudioGaterApp.h"

//==============================================================================

TransportBar::TransportBar(juce::AudioFormatManager& formatManager, juce::AudioTransportSource& audioSource) :
	audioThumbnailCache(5), audioThumbnail(2048, formatManager, audioThumbnailCache), transportSlider(audioSource)
{
	audioThumbnail.addChangeListener(this);
	addAndMakeVisible(transportSlider);
}

TransportBar::~TransportBar()
{
}

void TransportBar::paint(juce::Graphics& g)
{
	if (audioThumbnail.getNumChannels() == 0)
		paintIfNoFileLoaded(g);
	else
		paintIfFileLoaded(g);
}

void TransportBar::paintIfNoFileLoaded(juce::Graphics& g)
{
	g.fillAll(juce::Colours::lightblue);
	g.setColour(juce::Colours::darkgrey);
	g.drawFittedText("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
}

void TransportBar::paintIfFileLoaded(juce::Graphics& g)
{
	g.fillAll(juce::Colours::lightblue);

	g.setColour(juce::Colours::darkolivegreen);
	audioThumbnail.drawChannels(g, juce::Rectangle<int>(0, 0, getWidth(), getHeight() - 20), 0.0, audioThumbnail.getTotalLength(), 1.0f);
	transportSlider.setBounds(0, getHeight() - 20, getWidth(), 20);
}

void TransportBar::setSource(juce::File& file)
{
	audioThumbnail.setSource(new juce::FileInputSource(file));
}

void TransportBar::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &audioThumbnail)
		repaint();
}
