CFLAGS = -g -Wall -Wextra $(shell pkg-config --cflags rtmidi libevdev)
LDFLAGS =  $(shell pkg-config --libs rtmidi libevdev)

build: main.cpp facade_rt.cpp
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o keyboard_synthesizer
clean:
	rm -rf evdev-test

.PHONY: clean
