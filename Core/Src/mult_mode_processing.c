/*
 * mult_mode_processing.c
 *
 *  Created on: 14 ���. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

uint8_t mult_note_transpose(struct note_range *range, uint8_t note_number)
{
	uint8_t result = note_number;
	uint8_t temp_shift = range->parameters[MULTRANGESHIFTPos];
	if(temp_shift == 0) //collapse to low
	{
		result = range->parameters[MULTSTARTPos];
	}
	else if(temp_shift == 7)// collapse to high
	{
		result = range->parameters[MULTENDPos];
	}
	else if(temp_shift == 15) //inversion
	{
		result = 127 - note_number;
	}
	else if(temp_shift == 14) // stretch
	{
		uint8_t note_start = range->parameters[MULTSTARTPos];
		uint8_t note_end   = range->parameters[MULTENDPos];
		result =  note_start + (127-note_start)*(note_number - note_start)/(note_end - note_start);
	}
	else if(temp_shift < 7)
	{
		while(temp_shift > 0 && note_number > 12)
		{
			note_number -= 12;
			temp_shift--;
		}
		result = note_number;
	}
	else if(temp_shift > 8)
	{
		while(temp_shift > 8 && note_number < 115)
		{
			note_number += 12;
			temp_shift--;
		}
		result = note_number;
	}
	return result;
}


void send_to_mult_dest(struct note_range_array *range_array, struct midi_msg sending_note)
{
	struct midi_msg temp_note;
	midi_msg_init(&temp_note);

	for(int i = 0; i < MULTTIMBREAMOUNT; i++)
	{
		if(range_array->mtelement[i].parameters[MULTSTARTPos] <= sending_note.data1
		&& range_array->mtelement[i].parameters[MULTENDPos]   >= sending_note.data1 //note inside range
		&& range_array->mtelement[i].en   == 1)
		{
			// Change destination
			temp_note = sending_note;
			temp_note.status &= ~0xF;
			temp_note.status += range_array->mtelement[i].destination;

			// Change note octave
			temp_note.data1 = mult_note_transpose(&(range_array->mtelement[i]), temp_note.data1);
//			uint8_t temp_shift = range_array->mtelement[i].parameters[MULTRANGESHIFTPos];
//			if(temp_shift == 0) //collapse to low
//			{
//				temp_note.data1 = range_array->mtelement[i].parameters[MULTSTARTPos];
//			}
//			else if(temp_shift == 7)// collapse to high
//			{
//				temp_note.data1 = range_array->mtelement[i].parameters[MULTENDPos];
//			}
//			else if(temp_shift == 15) //inversion
//			{
//				temp_note.data1 = 127 - temp_note.data1;
//			}
//			else if(temp_shift == 14) // stretch
//			{
//				uint8_t note_start = range_array->mtelement[i].parameters[MULTSTARTPos];
//				uint8_t note_end   = range_array->mtelement[i].parameters[MULTENDPos];
//				temp_note.data1 =  note_start + (127-note_start)*(temp_note.data1 - note_start)/(note_end - note_start);
//			}
//			else if(temp_shift < 7)
//			{
//				while(temp_shift > 0 && temp_note.data1 > 12)
//				{
//					temp_note.data1 -= 12;
//					temp_shift--;
//				}
//			}
//			else if(temp_shift > 8)
//			{
//				while(temp_shift > 8 && temp_note.data1 < 115)
//				{
//					temp_note.data1 += 12;
//					temp_shift--;
//				}
//			}

			// Send to device
			if(temp_note.status >> 4 == 0x9
			&& range_array->mtelement[i].open == 1)
			{
				//display info
				if(range_array->mtelement[i].active != 0)
					range_array->mtelement[i].moving_toggles += 2;
				range_array->mtelement[i].active++;
				// sending
				midi_msg_to_synth(&temp_note);
			}
			else if(temp_note.status >> 4 == 0x8)
			{
				//display info
				if(range_array->mtelement[i].active > 1)
					range_array->mtelement[i].moving_toggles += 2;
				if(range_array->mtelement[i].active != 0)
					range_array->mtelement[i].active--;
				//sending
				//if(i != 3)
				midi_msg_to_synth(&temp_note);
			}

			//midi_msg_to_synth(&temp_note);

//			word_to_serial("mult: ");
//			midi_msg_print(&temp_note); // debug
		}
	}
}
