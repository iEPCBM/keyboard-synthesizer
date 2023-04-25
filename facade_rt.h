#ifndef FACADE_RT
#define FACADE_RT
#include <rtmidi/RtMidi.h>

#define FRT_SUCCESS 1
#define FRT_FAIL 0

class RtMidiFacade {
	public:
	RtMidiFacade();
	~RtMidiFacade();
	bool init();
	bool setMidiPort(int p);
	void programChange(char channel, char program);
	void setChannelVolume(char channel, char volume);
	void noteOn(char channel, char note_id, char volume);
	void noteOff(char channel, char note_id, char volume);
	void setSustain(char channel, bool on);
	RtMidiOut* getRtMidiOut();
	private:
	RtMidiOut *midiout = NULL;
};

#endif  FACADE_RT
