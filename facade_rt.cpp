#include <cstdlib>
#include <vector>
#include "facade_rt.h"

RtMidiFacade::RtMidiFacade() {}
RtMidiFacade::~RtMidiFacade() {
	if (midiout) {
		delete midiout;
		midiout = NULL;
	}
}

bool RtMidiFacade::init() {
	midiout = NULL;
	try {
		midiout = new RtMidiOut();
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		return false;
	}
	return true;
}

bool RtMidiFacade::setMidiPort(int p) {
	try {
		midiout->openPort(p);
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		return false;
	}
	return true;
}

void RtMidiFacade::programChange(char channel, char program) {
	std::vector<unsigned char> message;
	message = {
		0xC0|(channel&0x0F), program&0x7F
		};
	midiout->sendMessage( &message );
	
}

void RtMidiFacade::setChannelVolume(char channel, char volume) {
	std::vector<unsigned char> message;
	message = {
		0xB0|(channel&0x0F), 0x07, volume&0x7F
		};
	midiout->sendMessage( &message );
	
}

void RtMidiFacade::noteOff(char channel, char note_id, char volume) {
	std::vector<unsigned char> message;
	message = {
		0x80|(channel&0x0F), note_id&0x7F, volume&0x7F
		};
	midiout->sendMessage( &message );
	
}

void RtMidiFacade::noteOn(char channel, char note_id, char volume) {
	std::vector<unsigned char> message;
	message = {
		0x90|(channel&0x0F), note_id&0x7F, volume&0x7F
		};
	midiout->sendMessage( &message );
}

void RtMidiFacade::setSustain(char channel, bool on) {
	std::vector<unsigned char> message;
	char v = 0x00;
	if (on) {
		v = 0xFF;
	}
	message = {
		0xB0|(channel&0x0F), 0x40, v
		};
	midiout->sendMessage( &message );
}

RtMidiOut* RtMidiFacade::getRtMidiOut() {
	return midiout;
}
