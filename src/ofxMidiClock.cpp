/*
 *  ofxMidiClock.cpp
 *  SyncedMusicController
 *
 *  Created by Andreas Borg on 21/03/2015
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxMidiClock.h"

ofEvent<MidiClockEvent>	MidiClockEvent::TICK;
ofEvent<MidiClockEvent>	MidiClockEvent::BEAT;
ofEvent<MidiClockEvent>	MidiClockEvent::BAR;
ofEvent<MidiClockEvent>	MidiClockEvent::TIME_SIGNATURE;


ofEvent<MidiClockEvent>	MidiClockEvent::STOP;
ofEvent<MidiClockEvent>	MidiClockEvent::START;
ofEvent<MidiClockEvent>	MidiClockEvent::CONTINUE;


ofEvent<MidiClockEvent>	MidiClockEvent::MTC;