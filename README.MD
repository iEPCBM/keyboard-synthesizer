# keyboard-synthesizer
Play on PC keyboard(s) as on real synthesizer.

## Getting started
### Requirements
1. RtMidi (https://github.com/thestk/rtmidi)
2. libevdev

### Building
Just run:
	make build

### Usage
Run `./keyboard_synthesizer` and input session parameters:
	=== SETUP SESSION ===
	Enter MIDI port ID: <MIDI port>
	Enter base octave [0-10]: <octave>
	Enter keyboards count: <count>
	KB #0
		Enter channel ID [0-127]: <channel>
		Enter program ID [0-127]: <program (instrument)>
		Enter volume [0-127]: <volume>
		Enter octave offset [-3-7]: <offset>
		Enter dev event path: <path to event device (ex. /dev/input/event0)>
	...

P. S. Channel 0 always working with sustain.

## License
[MIT License](LICENSE)
