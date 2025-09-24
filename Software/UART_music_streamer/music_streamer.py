#!/usr/bin/env python3
"""
MIDI to BLDC Motor Audio Player
Converts MIDI files to frequency commands for STM32 FOC motor control
Sends real-time audio data via UART to play music on brushless motor
"""

import serial
import mido
import time
import numpy as np
from threading import Thread, Lock
from collections import deque
import argparse
import sys

class MIDIToMotorPlayer:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200, sample_rate=1000):
        self.serial_port = port
        self.baudrate = baudrate
        self.sample_rate = sample_rate  # Hz
        self.sample_interval = 1.0 / sample_rate
        
        # Motor parameters
        self.min_frequency = 20  # Hz - minimum audible frequency
        self.max_frequency = 5000  # Hz - maximum safe motor frequency
        self.base_amplitude = 0.3  # Base torque modulation amplitude
        
        # Audio state
        self.current_notes = {}  # Active notes: {note_number: (frequency, velocity, start_time)}
        self.playing = False
        self.serial_conn = None
        self.playback_thread = None
        self.audio_buffer = deque(maxlen=1000)
        self.buffer_lock = Lock()
        
        # MIDI note to frequency conversion (A4 = 440Hz)
        self.midi_reference = 69  # A4 MIDI note number
        self.a4_frequency = 440.0
        
    def midi_to_frequency(self, note_number):
        """Convert MIDI note number to frequency in Hz"""
        return self.a4_frequency * (2 ** ((note_number - self.midi_reference) / 12.0))
    
    def frequency_to_torque(self, frequency, amplitude=1.0):
        """Convert frequency to motor torque command"""
        # Normalize frequency to safe motor operating range
        if frequency < self.min_frequency:
            return 0.0
        
        # Scale amplitude based on frequency (prevent high-frequency damage)
        safe_amplitude = amplitude * min(1.0, 2000.0 / frequency)
        
        return safe_amplitude * self.base_amplitude
    
    def connect_serial(self):
        """Establish serial connection to STM32"""
        try:
            self.serial_conn = serial.Serial(
                port=self.serial_port,
                baudrate=self.baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=1
            )
            print(f"Connected to {self.serial_port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"Serial connection error: {e}")
            return False
    
    def send_motor_command(self, frequency, amplitude):
        """Send frequency command to motor via UART"""
        if not self.serial_conn:
            return False
        
        try:
            # Protocol: "FREQ:xxx.xx,AMP:y.yy\n"
            command = f"FREQ:{frequency:.2f},AMP:{amplitude:.3f}\n"
            self.serial_conn.write(command.encode('utf-8'))
            return True
        except Exception as e:
            print(f"Error sending command: {e}")
            return False
    
    def parse_midi_file(self, midi_file_path):
        """Parse MIDI file and extract note events with timing"""
        try:
            midi_file = mido.MidiFile(midi_file_path)
            events = []
            
            # Calculate ticks per second based on tempo
            ticks_per_beat = midi_file.ticks_per_beat
            default_tempo = 500000  # microseconds per beat (120 BPM)
            
            current_tempo = default_tempo
            absolute_time = 0  # in ticks
            tempo_changes = []
            
            # First pass: collect tempo changes
            for track in midi_file.tracks:
                track_time = 0
                for msg in track:
                    track_time += msg.time
                    if msg.type == 'set_tempo':
                        tempo_changes.append((track_time, msg.tempo))
            
            # Process all tracks
            for track in midi_file.tracks:
                track_time = 0
                
                for msg in track:
                    track_time += msg.time
                    absolute_time = track_time
                    
                    # Update tempo if needed
                    current_tempo = default_tempo
                    for change_time, tempo in tempo_changes:
                        if change_time <= absolute_time:
                            current_tempo = tempo
                    
                    # Convert ticks to seconds
                    ticks_per_second = (1_000_000 / current_tempo) * ticks_per_beat
                    timestamp = absolute_time / ticks_per_second
                    
                    if msg.type == 'note_on' and msg.velocity > 0:
                        frequency = self.midi_to_frequency(msg.note)
                        events.append({
                            'type': 'note_on',
                            'note': msg.note,
                            'frequency': frequency,
                            'velocity': msg.velocity / 127.0,  # Normalize to 0-1
                            'time': timestamp
                        })
                    elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
                        events.append({
                            'type': 'note_off',
                            'note': msg.note,
                            'time': timestamp
                        })
            
            # Sort events by time
            events.sort(key=lambda x: x['time'])
            return events
            
        except Exception as e:
            print(f"Error parsing MIDI file: {e}")
            return []
    
    def generate_audio_buffer(self, midi_events, duration):
        """Generate real-time audio buffer from MIDI events"""
        current_time = 0
        event_index = 0
        active_notes = {}
        
        while current_time < duration and event_index < len(midi_events):
            # Process events at current time
            while (event_index < len(midi_events) and (midi_events[event_index]['time'] <= current_time):
                event = midi_events[event_index]
                
                if event['type'] == 'note_on':
                    active_notes[event['note']] = {
                        'frequency': event['frequency'],
                        'amplitude': event['velocity'],
                        'start_time': current_time
                    }
                elif event['type'] == 'note_off' and event['note'] in active_notes:
                    del active_notes[event['note']]
                
                event_index += 1
            
            # Calculate combined signal from active notes (simple additive synthesis)
            if active_notes:
                # For polyphonic music, we can mix multiple frequencies
                # Here we use the highest amplitude note or average frequency
                total_amplitude = sum(note['amplitude'] for note in active_notes.values())
                if total_amplitude > 0:
                    # Weighted average frequency based on amplitude
                    weighted_freq = sum(note['frequency'] * note['amplitude'] 
                                      for note in active_notes.values()) / total_amplitude
                    amplitude = min(1.0, total_amplitude)  # Clamp amplitude
                else:
                    weighted_freq = 0
                    amplitude = 0
            else:
                weighted_freq = 0
                amplitude = 0
            
            # Add to buffer
            with self.buffer_lock:
                self.audio_buffer.append({
                    'time': current_time,
                    'frequency': weighted_freq,
                    'amplitude': amplitude
                })
            
            current_time += self.sample_interval
        
        print(f"Generated {len(self.audio_buffer)} audio samples")
    
    def playback_loop(self):
        """Main playback loop that sends commands to motor"""
        start_time = time.time()
        buffer_index = 0
        
        print("Starting playback...")
        
        while self.playing and buffer_index < len(self.audio_buffer):
            current_time = time.time() - start_time
            
            # Find the appropriate sample for current time
            with self.buffer_lock:
                if buffer_index < len(self.audio_buffer):
                    sample = self.audio_buffer[buffer_index]
                    
                    # If it's time to play this sample
                    if current_time >= sample['time']:
                        frequency = sample['frequency']
                        amplitude = sample['amplitude']
                        
                        if frequency > 0:
                            torque = self.frequency_to_torque(frequency, amplitude)
                            self.send_motor_command(frequency, torque)
                        else:
                            # Silence - send zero torque
                            self.send_motor_command(0, 0)
                        
                        buffer_index += 1
            
            # Maintain sample rate
            time.sleep(self.sample_interval)
        
        # Send stop command at end
        self.send_motor_command(0, 0)
        print("Playback finished")
    
    def play_midi_file(self, midi_file_path):
        """Main function to play MIDI file on motor"""
        if not self.connect_serial():
            return False
        
        print(f"Loading MIDI file: {midi_file_path}")
        midi_events = self.parse_midi_file(midi_file_path)
        
        if not midi_events:
            print("No events found in MIDI file")
            return False
        
        # Calculate total duration
        duration = max(event['time'] for event in midi_events) + 1.0  # Add 1 second padding
        
        print(f"MIDI file duration: {duration:.2f} seconds")
        print(f"Number of events: {len(midi_events)}")
        
        # Generate audio buffer
        print("Generating audio buffer...")
        self.generate_audio_buffer(midi_events, duration)
        
        # Start playback
        self.playing = True
        self.playback_thread = Thread(target=self.playback_loop)
        self.playback_thread.start()
        
        # Wait for playback to complete
        try:
            self.playback_thread.join()
        except KeyboardInterrupt:
            self.stop_playback()
        
        return True
    
    def play_scale(self):
        """Play a simple musical scale for testing"""
        if not self.connect_serial():
            return False
        
        # C Major scale frequencies (C4 to C5)
        scale_frequencies = [261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25]
        note_duration = 0.5  # seconds
        
        self.playing = True
        print("Playing C Major scale...")
        
        for freq in scale_frequencies:
            if not self.playing:
                break
            
            # Play note
            torque = self.frequency_to_torque(freq, 0.7)
            self.send_motor_command(freq, torque)
            time.sleep(note_duration)
            
            # Brief silence between notes
            self.send_motor_command(0, 0)
            time.sleep(0.05)
        
        # Final silence
        self.send_motor_command(0, 0)
        self.playing = False
        print("Scale playback complete")
        return True
    
    def stop_playback(self):
        """Stop current playback"""
        self.playing = False
        if self.serial_conn:
            self.send_motor_command(0, 0)  # Ensure motor stops
        print("Playback stopped")

def main():
    parser = argparse.ArgumentParser(description='MIDI to BLDC Motor Player')
    parser.add_argument('--port', '-p', default='/dev/ttyUSB0', 
                       help='Serial port (default: /dev/ttyUSB0)')
    parser.add_argument('--baud', '-b', type=int, default=115200,
                       help='Baud rate (default: 115200)')
    parser.add_argument('--scale', '-s', action='store_true',
                       help='Play test scale instead of MIDI file')
    parser.add_argument('midi_file', nargs='?', help='MIDI file to play')
    
    args = parser.parse_args()
    
    player = MIDIToMotorPlayer(port=args.port, baudrate=args.baud)
    
    try:
        if args.scale:
            player.play_scale()
        elif args.midi_file:
            player.play_midi_file(args.midi_file)
        else:
            print("Please specify a MIDI file or use --scale for test scale")
            parser.print_help()
    
    except KeyboardInterrupt:
        print("\nInterrupted by user")
        player.stop_playback()
    
    except Exception as e:
        print(f"Error: {e}")
        player.stop_playback()

if __name__ == "__main__":
    main()