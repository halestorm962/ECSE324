#include "./drivers/inc/vga.h"
#include "./drivers/inc/ISRs.h"
#include "./drivers/inc/LEDs.h"
#include "./drivers/inc/audio.h"
#include "./drivers/inc/HPS_TIM.h"
#include "./drivers/inc/int_setup.h"
#include "./drivers/inc/wavetable.h"
#include "./drivers/inc/pushbuttons.h"
#include "./drivers/inc/ps2_keyboard.h"
#include "./drivers/inc/HEX_displays.h"
#include "./drivers/inc/slider_switches.h"

int volume = 1;

int signal(float f, int t) {
	int temp = (int)(f*t);
	int index = temp % 48000;				// TODO: make this a float
	int indexLeftOfDecimal = (int)index;
	float decimals = index - indexLeftOfDecimal;
	float interpolated = (1-decimals)*sine[indexLeftOfDecimal] + (decimals)*sine[indexLeftOfDecimal+1];

	// TODO: replace amplitude here with the volume control
	return volume * interpolated;
}

void wave(float f) {
	int x, y;
	short colour = 0;
	// iterate through all of the pixels on the screen
	for(y=0; y<=239; y++) {
		for(x=0; x<=319; x++) {
			// TODO: only draw if that pixel is part of the sin wave
			VGA_draw_point_ASM(x, y, colour++);
		}
	}
}

int main() {
	int samples = 0;
	char* data;		// PS/2 port address
	float f = 0;	// frequency of note to play
					// TODO: start at f = 0
	int clock = 0;
	int delay = 1;
	char current = 0;
	char previous = 0;

	// TODO: handle more than one keypress

	while(1) {
		int output = 0;
		char input = *data;
		// if the RVALID flag is 1, enter this if block
		if (read_ps2_data_ASM(data)) {
			// typematic situation requires more logic, so put it here separately
			if (input == current) {
				if (previous == 0xF0 || previous == 0xFE || previous == 0xFA) {
					// if the previous is a break code, we have a recurring keystroke
						// register the keystroke and update our list
					previous = current;
					current = input;
				} else {
					// otherwise, a key is being held, so start the typematic process
					
					// if input data is the current input, do nothing (i.e. can only hit each key once)
					clock += 1;		// increment "timer"
					
					// if we're still in the typematic delay phase, wait until the "clock" gets to 20
					if (delay) {
						if (clock < 8) {
							clock++;		// stay in typematic delay
						} else {
							clock = 0;						// reset "clock"
							delay = 0;						// indicate that we've left the typematic delay phase
						}
					} else {
						// if we're out of the typematic delay phase, implement a shorter delay for rapid key output
						if (clock < 2) {
							clock++;
						} else {
							clock = 0;						// reset "clock"
						}
					}
				}
			} else {
				// reset typematic variables
				clock = 0;
				delay = 1;

				// all other cases go here
				if (input == 0) {
					// if input data is empty, no keys are pressed, so set frequency to 0
					output = 2;
				} else if (input == 0xF0 || input == 0xFE || input == 0xFA) {
					// if input data is a break code, a key has been released
					// update our list
					previous = current;
					current = input;

					// output = 2;		// the key has been released, so set the frequency to zero
				} else if (input == previous) {
					// the input is the same as two codes ago and NOT the same as one code ago
						// this must be the code automatically sent right after the break code
					// current status is: previous = key, current = break code, input = same key
						// we need to update our list
					previous = current;
					current = input;
				} else {
					// input is a new key press

					// if the previous value is a break code, the prior key was released
					if (previous == 0xF0 || previous == 0xFE || previous == 0xFA) {
						output = 1;
					} else {
						// (at least) 2 keys are simultaneously pressed
						output = 3;
					}

					// update list
					previous = current;
					current = input;
				}
			}
		}


		// frequency adjustment
		if (output == 1) {
			if (input == 0x2A) {
				// volume up
				volume++;
			} else if (input == 0x21) {
				// volume down
				volume--;
			} else if (input == 0x1C) {
				// hit A, play a C
				f = 130.813;
			} else if (input == 0x1B) {
				// hit S, play a D
				f = 146.832;
			} else if (input == 0x23) {
				// hit D, play an E
				f = 164.814;
			} else if (input == 0x2B) {
				// hit F, play an F
				f = 174.614;
			} else if (input == 0x3B) {
				f = 195.998;
			} else if (input == 0x42) {
				f = 220.000;
			} else if (input == 0x4B) {
				f = 246.942;
			} else if (input == 0x4C) {
				f = 261.626;
			} else {
				f = 0;
			}
			output = 0;
		} else if (output == 2) {
			f = 0;
			output = 0;
		} else if (output == 3) {
			// volume adjustment
			if (input == 0x2A) {
				// volume up
				volume++;
			} else if (input == 0x21) {
				// volume down
				volume--;
			} else {
				// frequency adjustment
				float f1 = 0;
				if (input == 0x1C) {
					// hit A, play a C
					f1 = 130.813;
				} else if (input == 0x1B) {
					// hit S, play a D
					f1 = 146.832;
				} else if (input == 0x23) {
					// hit D, play an E
					f1 = 164.814;
				} else if (input == 0x2B) {
					// hit F, play an F
					f1 = 174.614;
				} else if (input == 0x3B) {
					f1 = 195.998;
				} else if (input == 0x42) {
					f1 = 220.000;
				} else if (input == 0x4B) {
					f1 = 246.942;
				} else if (input == 0x4C) {
					f1 = 261.626;
				} else {
					f1 = 0;
				}
				f = (f+f1)/2;
			}
			output = 0;
		}

		// display wave to screen
		// wave(f);		// can't run this every time - if we do, it's too slow!

		// if frequency is 0, don't play anything
		if (f) {
			// generate audio sample
			int endOfSignal = 48000 / f;
			while (samples < endOfSignal) {		// iterate one period
				// send a value
				int s = signal(f, samples);
				if (audio_write_data_ASM(s, s)) {						// TODO: decide whether keeping or removing this if condition changes the tune
					samples++;	// increment number of samples sent
				}
			}
			samples = 0;
		}
	}

	return 0;
}
