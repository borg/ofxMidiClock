/*
 *  ofxMidiClock.h
 *  SyncedMusicController
 *
 *  Translate MIDI_SONG_POS_POINTER, MIDI_TIME_CLOCK, MIDI_START, MIDI_CONTINUE, MIDI_STOP into something useful
 *  
 *  In the interest of brevity these midi messages don't readily contain the musical information. It needs to be
 *  extracted with a rhythm stick. You will notice that some scrubbing is needed to get your DAW and oF to latch on.
 *
 
 *  You need to explicitely set time signature.
 *  You can either add this as a listener to midi events
 *  or just pass the event itself on from your main listener
 
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
 
 *
 *  Created by Andreas Borg on 21/03/2015
 *  Copyright 2015 http://crea.tion.to All rights reserved.
 *
 */

#ifndef _ofxMidiClock
#define _ofxMidiClock

#include "ofMain.h"
#include "ofxMidi.h"




#define kMTCFrames      0
#define kMTCSeconds     1
#define kMTCMinutes     2
#define kMTCHours       3





class MidiClockEvent : public ofEventArgs
{
    
public:
    
    MidiClockEvent()
    {
        //CLOCK
        tick = 0;
        beat = 1;
        bar = 1;
        timeNumerator = 4;
        timeDenominator = 4;
        
        
        
        //MTC
        hours			= 0;
        minutes			= 0;
        seconds			= 0;
        frames			= 0;
        secondFraction  = 0.0f;
        
        numFrames		= 25;
        
        timeAsMillis 	= 0;
    }
    
    
    //CLOCK
    int tick;
    int beat;
    int bar;
    int timeNumerator;
    int timeDenominator;
    
    
    
    //MTC
    int hours;
    int minutes;
    int seconds;
    int frames;
    float secondFraction;
    
    int numFrames;
    
    int timeAsMillis;
    
    
    static ofEvent<MidiClockEvent>	TICK;
    static ofEvent<MidiClockEvent>	BEAT;
    static ofEvent<MidiClockEvent>	BAR;
    static ofEvent<MidiClockEvent>	TIME_SIGNATURE;
    
    static ofEvent<MidiClockEvent>	STOP;
    static ofEvent<MidiClockEvent>	START;
    static ofEvent<MidiClockEvent>	CONTINUE;
    
    
    static ofEvent<MidiClockEvent>	MTC;
};




class ofxMidiClock {
	
  public:
	
    ofxMidiClock(){
        timeNumerator = 4;//beats per bar
        timeDenominator = 4;//beat unit
        
        currTick = 0;
        currBar = 1;
        currBeat = 1;
        oldBeat = 1;
        total24thTicks = 24;
        total16thTicks = 0;
        
        
        isVerbose = 0;
    
    };
	
    int getBeat(){
        return currBeat;
    }
    
    int getBar(){
        return currBar;
    }
    
    int getTick(){
        return currTick;
    }
    
    int getTimeNumerator(){
        return timeNumerator;
    }
    
    int getTimeDenominator(){
        return timeDenominator;
    }
    
    void setVerbos(bool b){
        isVerbose = b;
    }
    
    
    //you can only call this once each frame if you are checking in say update/draw
    //alternatively use a listener
    bool isNewBeat(){
        if(oldBeat != currBeat){
            oldBeat = currBeat;
            return true;
        }else{
            return false;
        }
        
    }
    
    void setTimeSignature(int _numerator, int _denominator){
        timeNumerator = _numerator;
        timeDenominator = _denominator;
        
        MidiClockEvent e;
        e.tick = currTick;
        e.timeNumerator = timeNumerator;
        e.timeDenominator = timeDenominator;
        e.beat = currBeat;
        e.bar = currBar;
        ofNotifyEvent(MidiClockEvent::TIME_SIGNATURE,e);
        
        
    };
    

    void newMidiMessage(ofxMidiMessage& msg){
        
        
        MidiClockEvent e;
        //These values may be adjusted below before sent, but these are current defaults
        //CLOCK
        e.tick = currTick;
        e.timeNumerator = timeNumerator;
        e.timeDenominator = timeDenominator;
        e.beat = currBeat;
        e.bar = currBar;
        
        //MTC
        e.hours = timcodeEventArgs.hours;
        e.minutes = timcodeEventArgs.minutes;
        e.seconds = timcodeEventArgs.seconds;
        e.frames = timcodeEventArgs.frames;
        e.secondFraction = timcodeEventArgs.secondFraction;
        e.timeAsMillis = timcodeEventArgs.timeAsMillis;
        
        
    
        if(msg.status == MIDI_SONG_POS_POINTER){
            
            /*
            time clock counts 16th ticks from beginning
            
            
            divide by 4 to get quarter notes
            http://www.recordingblogs.com/sa/tabid/88/Default.aspx?topic=MIDI%20Song%20Position%20Pointer%20message
            
            this article is wrong...at least Logic sends the sencond byte first
            
            I'm sure you can do this nicely with bitwise operators, but it's not my forte
            
            Essentially the message is made up of 3 bytes,
             first is the status msg, second and third 8 bits each to
             be combined to a 14 bit long binary number by chopping off first
             bits and shifting the third byte seven steps to the left
             
             
             */
            
            unsigned short one = (int)  msg.bytes.at(1);
            string bin1 = ofToBinary(one);
            
            unsigned short two = (int)  msg.bytes.at(2);
            
            string bin2 = ofToBinary(two);
            
            //combine the 7 out of 8 binary bit, to one long 14 bit
            string combineBytes = bin2.substr(bin2.size()-7)+bin1.substr(bin1.size()-7);
            total16thTicks = ofBinaryToInt(combineBytes);
            
            
            //each beat contains 4 16th note
            currBar = (total16thTicks*(timeDenominator/4))/(4*timeNumerator)+1;
            currBeat = (total16thTicks*(timeDenominator/4)-(currBar-1)*(4*timeNumerator))/4+1;
            
            int totalBeats = (currBar-1)*timeNumerator+currBeat-1;
            
            
            currTick = totalBeats*total24thTicks *4/(float)timeDenominator;
            
            if(isVerbose){
                ofLog()<<"MIDI_SONG_POS_POINTER adjustment: "<<" currBar : "<<currBar<<" currBeat "<<currBeat<<" total16thTicks "<<total16thTicks<<" currTick "<<currTick<<" totalBeats "<<totalBeats<<endl;
            }
            ofNotifyEvent(MidiClockEvent::BAR,e);
            ofNotifyEvent(MidiClockEvent::BEAT,e);
            ofNotifyEvent(MidiClockEvent::TICK,e);
            
            
            
        }else if(msg.status == MIDI_TIME_CLOCK){
            
            //The MIDI clock is a single status byte (F8) to be sent 24 times per MIDI beat
            
            /*
             So, what exactly is MIDI clock? Well, it is a set of specific bytes that form part of the MIDI protocol. The great thing is that MIDI clock messages are only one byte long (instead of two or three bytes like other MIDI messages), so the code is very simple.
             
             Here are the main, useful MIDI clock bytes:
             
             start = 0xfa
             stop = 0xfc
             clock tick = 0xf8
             continue = 0xfb
             
             Twenty-four clock tick bytes are sent per quarter note when the host sequencer is running. Every time the sequencer is stopped, the stop byte is sent. Every time the sequencer is started after being stopped, the start byte is sent. Every time the sequencer is started after being paused, the continue byte is sent.
             */
            
            currTick++;
            
            
            e.tick = currTick;
            
            if(currTick*(timeDenominator/4) % total24thTicks == 0 ) {
                
                currBeat = (currTick*(timeDenominator/4)/total24thTicks)%timeNumerator+1;
                e.beat = currBeat;
                
                
                if(currBeat == 1){
                    currBar++;
                    
                    e.bar = currBar;
                    
                    ofNotifyEvent(MidiClockEvent::BAR,e);
                }
                
                ofNotifyEvent(MidiClockEvent::BEAT,e);
                ofNotifyEvent(MidiClockEvent::TICK,e);
                if(isVerbose){
                    ofLog()<<"MIDI_TIME_CLOCK "<<currBar <<" : "<<currBeat<<endl;
                }
            }
            
        }else if(msg.status == MIDI_CONTINUE){
            if(isVerbose){
                ofLog()<<"MIDI_CONTINUE RECEIVED"<<endl;
            }
            //because pause/play/continue are asyncronous they mess with things...I'm sure
            //this can be improved
            //every time it is being called the song pos seems to be called as well,
            //so should be able to sync them
            ofNotifyEvent(MidiClockEvent::CONTINUE,e);

            
        }else if(msg.status == MIDI_STOP){
            if(isVerbose){
                ofLog()<<"MIDI_STOP RECEIVED"<<endl;
            }
            
            ofNotifyEvent(MidiClockEvent::STOP,e);

        }else if(msg.status == MIDI_START){
            if(isVerbose){
                ofLog()<<"MIDI_START RECEIVED"<<endl;
            }
            
            //needs to sync when scrubbed or restarted
            oldBeat = 1;
            currTick = 1;
            currBar = 1;
            currBeat=1;
            
            
            e.tick = currTick;
            e.beat = currBeat;
            e.bar = currBar;
            ofNotifyEvent(MidiClockEvent::START,e);

        }else if( msg.status == MIDI_TIME_CODE &&		// if this is a MTC message...
           msg.bytes.size() > 1  )
        {
            
            
            timcodeEventArgs.tick = currTick;
            timcodeEventArgs.timeNumerator = timeNumerator;
            timcodeEventArgs.timeDenominator = timeDenominator;
            timcodeEventArgs.beat = currBeat;
            timcodeEventArgs.bar = currBar;
            
            
            
            
            // these static variables could be globals, or class properties etc.
            static int times[4]     = {0, 0, 0, 0};                 // this static buffer will hold our 4 time componens (frames, seconds, minutes, hours)
            //static char *szType     = "";                           // SMPTE type as string (24fps, 25fps, 30fps drop-frame, 30fps)
            static int numFrames    = 100;                          // number of frames per second (start off with arbitrary high number until we receive it)
            
            int messageIndex        = msg.bytes.at(1) >> 4;       // the high nibble: which quarter message is this (0...7).
            int value               = msg.bytes.at(1) & 0x0F;     // the low nibble: value
            int timeIndex           = messageIndex>>1;              // which time component (frames, seconds, minutes or hours) is this
            bool bNewFrame          = messageIndex % 4 == 0;
            
            
            // the time encoded in the MTC is 1 frame behind by the time we have received a new frame, so adjust accordingly
            if(bNewFrame) {
                times[kMTCFrames]++;
                if(times[kMTCFrames] >= numFrames) {
                    times[kMTCFrames] %= numFrames;
                    times[kMTCSeconds]++;
                    if(times[kMTCSeconds] >= 60) {
                        times[kMTCSeconds] %= 60;
                        times[kMTCMinutes]++;
                        if(times[kMTCMinutes] >= 60) {
                            times[kMTCMinutes] %= 60;
                            times[kMTCHours]++;
                        }
                    }
                }
                
                //printf("%i:%i:%i:%i | %s\n", times[3], times[2], times[1], times[0], szType);
                
                timcodeEventArgs.hours   = times[3];
                timcodeEventArgs.minutes = times[2];
                timcodeEventArgs.seconds = times[1];
                timcodeEventArgs.frames  = times[0];
                
                timcodeEventArgs.secondFraction = (float)timcodeEventArgs.frames / (float)timcodeEventArgs.numFrames;
                
                timcodeEventArgs.timeAsMillis = timeToMillis( timcodeEventArgs );
                
                ofNotifyEvent( MidiClockEvent::MTC, timcodeEventArgs);
                
                //sprintf( reportString, "%i:%i:%i:%i | %s\n", times[3], times[2], times[1], times[0], szType );
            }
            
            if( timeIndex < 0 || timeIndex > 3 )
            {
                ofLogError() << "ofxMTCReceiver::newmsg,  timeIndex should not be able to reach " << timeIndex;
                timeIndex %= 3;
            }
            
            if(messageIndex % 2 == 0) {                             // if this is lower nibble of time component
                times[timeIndex]    = value;
            } else {                                                // ... or higher nibble
                times[timeIndex]    |=  value<<4;
            }
            
            
            if(messageIndex == 7) {
                times[kMTCHours] &= 0x1F;                               // only use lower 5 bits for hours (higher bits indicate SMPTE type)
                int smpteType = value >> 1;
                switch(smpteType) {
                    case 0: numFrames = 24; /*szType = "24 fps";*/ break;
                    case 1: numFrames = 25; /*szType = "25 fps";*/ break;
                    case 2: numFrames = 30; /*szType = "30 fps (drop-frame)";*/ break;
                    case 3: numFrames = 30; /*szType = "30 fps";*/ break;
                    default: numFrames = 100; /*szType = " **** unknown SMPTE type ****";8*/
                }
                
                timcodeEventArgs.numFrames = numFrames;
            }
        }
        
        
        
    };
    
    

    
    
    
    static int timeToMillis( MidiClockEvent _args ){
        return timeToMillis( _args.hours, _args.minutes, _args.seconds, _args.secondFraction * 1000 );
    }
    
    // ---------------------------------------------------------------------------------------------------------------------------------------------------
    //
    static int timeToMillis( int _hour, int _minutes, int _seconds, int _millis ){
        int tmpMillis = 0;
        
        tmpMillis += _hour    * (1000*60*60);
        tmpMillis += _minutes * (1000*60);
        tmpMillis += _seconds * (1000);
        tmpMillis += _millis;
        
        return tmpMillis;
    }
    
    // ---------------------------------------------------------------------------------------------------------------------------------------------------
    //
    static string timeAsString( int _milliSeconds ){
        int milliseconds = (int) _milliSeconds % 1000;
        int seconds = (int) ( _milliSeconds /  1000) % 60 ;
        int minutes = (int) ((_milliSeconds / (1000*60)) % 60);
        int hours   = (int) ((_milliSeconds / (1000*60*60)) % 24);
        
        //ofLog() << ":" << hours << ":" << minutes << ":" << seconds << ":" << milliseconds << endl;
        
        string timeAsString = "";
        if( hours < 10 ) timeAsString = timeAsString + "0";
        timeAsString = timeAsString + ofToString(hours);
        timeAsString = timeAsString + ":";
        
        if( minutes < 10 ) timeAsString = timeAsString + "0";
        timeAsString = timeAsString + ofToString(minutes);
        timeAsString = timeAsString + ":";
        
        if( seconds < 10 ) timeAsString = timeAsString + "0";
        timeAsString = timeAsString + ofToString(seconds);
        timeAsString = timeAsString + ":";
        
        if( milliseconds < 1000 ) timeAsString = timeAsString + "0";
        if( milliseconds < 100 )  timeAsString = timeAsString + "0";
        if( milliseconds < 10 )   timeAsString = timeAsString + "0";
        
        timeAsString = timeAsString + ofToString(milliseconds);	
        
        return timeAsString;
    }
    
protected:
    int timeNumerator;
    int timeDenominator;
    
    int total16thTicks;//from midi song position
    int total24thTicks;//from midi clock
    int ticksPerBeat;
    int currTick;
    int currBar;
    int oldBeat;
    int currBeat;
    
    
    bool isVerbose;
    
    
    MidiClockEvent			timcodeEventArgs;
    
    
    
    // trim from both ends
    static inline std::string &trim(std::string &s)  {
        return ltrim(rtrim(s));
    }
    
    // trim from start
    static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }
    
    // trim from end
    static inline std::string &rtrim(std::string &s)  {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }
    

    
};

#endif
