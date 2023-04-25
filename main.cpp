#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "facade_rt.h"
#include <linux/input.h>
#include <cstdio>
#include <fcntl.h>
#include <cstring>

#include <libevdev/libevdev.h>


using namespace std;

#define KEYS_NUM 34
#define FLUSH_ID 33
#define KEY_EVDEV_SUCCESS 0
static const uint8_t key_value_map[] = {
    KEY_Z,    // 0x00
    KEY_S,    // 0x01
    KEY_X,    // 0x02
    KEY_D,    // 0x03
    KEY_C,    // 0x04
    KEY_V,    // 0x05
    KEY_G,    // 0x06
    KEY_B,    // 0x07
    KEY_H,    // 0x08
    KEY_N,    // 0x09
    KEY_J,    // 0x0A
    KEY_M,    // 0x0B
    KEY_Q,    // 0x0C
    KEY_2,    // 0x0D
    KEY_W,    // 0x0E
    KEY_3,    // 0x0F
    KEY_E,    // 0x10
    KEY_R,    // 0x11
    KEY_5,    // 0x12
    KEY_T,    // 0x13
    KEY_6,    // 0x14
    KEY_Y,    // 0x15
    KEY_7,    // 0x16
    KEY_U,    // 0x17
    KEY_I,    // 0x18
    KEY_9,    // 0x19
    KEY_O,    // 0x1A
    KEY_0,    // 0x1B
    KEY_P,    // 0x1C
    KEY_LEFTBRACE,
    KEY_EQUAL,
    KEY_RIGHTBRACE,
    KEY_BACKSPACE,
    KEY_ESC
};


struct libevdev_t {
	libevdev* dev;
};

struct kb_evt_rel_ocatve_t {
	int relOctave;
	string kbEvtPath;
	libevdev_t * ke = NULL;
	
	unsigned int channel;
	unsigned int program;
	unsigned int volume;
};
	
struct setup_params_t {
	int iMidiPort;
	unsigned int baseOctave;
	
	vector<kb_evt_rel_ocatve_t> kbEvtPaths;
};


size_t getIndex(uint8_t val) {
	for(size_t i=0; i<KEYS_NUM; i++) {
		if (val == key_value_map[i])
			return i;
	}
	return KEYS_NUM;
}

int key_evdev_new(const char *path, libevdev_t **ke_ptr)
{
    int rc = 1;
    int fd = 0;
    libevdev *dev = NULL;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        cout<<"Failed to open device " << path << endl;
        return rc;
    }
    libevdev_t *ke = new libevdev_t;
	ke->dev = dev;
    rc = libevdev_new_from_fd(fd, &ke->dev);
    *ke_ptr = ke;

    return rc;
}

void evdev_resync(libevdev_t *ke)
{
	int rc = -1;
    do {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
    } while (rc == LIBEVDEV_READ_STATUS_SYNC);
}

int key_evdev_flush(libevdev_t *ke)
{
    int rc = -1;

    while (rc != -EAGAIN) {
        struct input_event ev;
        rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC) {
            evdev_resync(ke);
        }
    }
	return KEY_EVDEV_SUCCESS;
}

void key_evdev_free(libevdev_t *ke)
{
    if (!ke)
        return;
    libevdev_free(ke->dev);
    delete ke;
}

setup_params_t init_setup() {
	setup_params_t retval;
	cout<<"=== SETUP SESSION ==="<<endl;
	cout<<"Enter MIDI port ID: ";
	cin>>retval.iMidiPort;
	
	
	do {
		cout<<"Enter base octave [0-10]: ";
		cin>>retval.baseOctave;
	} while (retval.baseOctave > 10);
	
	unsigned int d;
	cout<<"Enter keyboards count: ";
	cin>>d;
	
	int mino, maxo;
	maxo = 10 - retval.baseOctave;
	mino = -retval.baseOctave;
	
	for (size_t i=0; i<d; i++) {
		kb_evt_rel_ocatve_t ko;
		cout<<"KB #"<<i<<endl;
		do {
			cout<<"\tEnter channel ID [0-127]: ";
			cin>>ko.channel;
		} while (ko.channel > 127);
		
		do {
			cout<<"\tEnter program ID [0-127]: ";
			cin>>ko.program;
		} while (ko.program > 127);

		
		do {
			cout<<"\tEnter volume [0-127]: ";
			cin>>ko.volume;
		} while (ko.volume > 127);
		
		do {
			cout<<"\tEnter octave offset ["<<mino<<"-"<<maxo<<"]: ";
			cin>>ko.relOctave;
		} while (ko.relOctave < mino || ko.relOctave > maxo);
		cout<<"\tEnter dev event path: ";
		cin>>ko.kbEvtPath;
		retval.kbEvtPaths.push_back(ko);
	}
	cout << "=== DONE ===" << endl;
	return retval;
}


int main(void)
{
	setup_params_t params = init_setup();
	
	
    int rc = 1;
    
	for (size_t i=0; i<params.kbEvtPaths.size(); i++) {
		cout<< "Connecting "<<params.kbEvtPaths[i].kbEvtPath.c_str()<<endl;
		rc = key_evdev_new(params.kbEvtPaths[i].kbEvtPath.c_str(), &params.kbEvtPaths[i].ke);
		if (rc != KEY_EVDEV_SUCCESS) {
			cout<<"Unable to connect device "<<params.kbEvtPaths[i].kbEvtPath.c_str()<<endl;
			return -1;
		}
	}

    sleep(1);
    cout<<"Starting MIDI module...\n";
    RtMidiFacade facade;
	facade.init();
	facade.setMidiPort(params.iMidiPort);
	
	for (size_t i=0; i<params.kbEvtPaths.size(); i++) {
		facade.programChange(params.kbEvtPaths[i].channel, params.kbEvtPaths[i].program);
		facade.setChannelVolume(params.kbEvtPaths[i].channel, params.kbEvtPaths[i].volume);
	}
	

	bool isPlaying = true;
	cout<<"READY\n";

    while(isPlaying) {
		for (size_t i=0; i<params.kbEvtPaths.size(); i++) {
			libevdev_t *ke = params.kbEvtPaths[i].ke;
			struct input_event ev;
			rc = libevdev_next_event(ke->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
	
			if (rc == LIBEVDEV_READ_STATUS_SYNC) {
				evdev_resync(ke);
			} else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
				if (ev.type == EV_KEY && ev.code != KEY_RESERVED) {
					int index = getIndex(ev.code);
					cout<<"Input: "<<ev.code<<"\tIndex: "<<index<<endl;

					if (index==FLUSH_ID)
						isPlaying = false;
					else if (index==KEYS_NUM)
						continue;
					else {
						int note_id = getIndex(ev.code)+12*(params.baseOctave+params.kbEvtPaths[i].relOctave);
						if (ev.value == 1) {
							facade.noteOn(params.kbEvtPaths[i].channel, note_id&0x7F, params.kbEvtPaths[i].volume);
						}
						else if (ev.value == 0 && params.kbEvtPaths[i].channel != 0) {
							facade.noteOff(params.kbEvtPaths[i].channel, note_id&0x7F, params.kbEvtPaths[i].volume);
						}
					}
				}
			} else if (rc != -EAGAIN) {
				cout<<"WARN: "<<rc<<endl;
			}
		}
    }

    cout<<"Flushing all...\n";

    for (size_t i=0; i<params.kbEvtPaths.size(); i++) {
		rc = key_evdev_flush(params.kbEvtPaths[i].ke);
		if (rc != KEY_EVDEV_SUCCESS){
			cerr<<"Failed to flush device "<<params.kbEvtPaths[i].kbEvtPath;
			return 1;
		}
		key_evdev_free(params.kbEvtPaths[i].ke);
	}
    return 0;
}

