# ofxMidiClock
Translate MIDI_SONG_POS_POINTER, MIDI_TIME_CLOCK, MIDI_START, MIDI_CONTINUE, MIDI_STOP into something useful



In the interest of brevity these midi messages don't readily contain the musical information. It needs to be extracted with a rhythm stick. You will notice that some scrubbing is needed to get your DAW and oF to latch on.

 
 Usage:
 *  You need to explicitely set time signature.
 *  You can either add this as a listener to midi events or just pass the event itself on from your main listener
 
 void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    midiClock.newMidiMessage(msg);
 }
 
 
In your app listen to MidiClockEvent
ofAddListener(MidiClockEvent::BEAT, this, &ofApp::onLogicBeat);
ofAddListener(MidiClockEvent::MTC, this, &ofApp::onLogicTimer);
 
 void ofApp::onLogicTimer(MidiClockEvent &e){
    midiMinutes = e.minutes;
    midiSeconds = e.seconds;
    midiFrames = e.frames;
    midiMillis = e.timeAsMillis;
 };
 
 
To get the midi info from Logic you need to enable the send in the midi sync preferences inside Logic
 
This is also consolidating ofxMTCReceiver by Andreas Muller, based on Memo's analysis, into one addon
 
 
Created by Andreas Borg on 21/03/2015
Copyright 2015 http://crea.tion.to 
All rights reserved or whatever.
