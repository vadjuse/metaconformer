/*
 * sequencer_mode_actions.c
 *
 *  Created on: 3 ���. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"


extern uint8_t gate_interval;
extern struct midi_msg incoming_note_buffer;
extern uint8_t learn_seq_trigger_toggles;
extern int midi_access_semaph;


void message_append_to_sequencer(struct midi_container *learning_sequencer)
{
	word_to_serial(" interval: \r");									//debug
	number_to_serial_human_readable(gate_interval);						//debug

	learning_sequencer->note[learning_sequencer->last_index] = incoming_note_buffer;  	//append midi note

	//while(midi_access_semaph == 1);
	midi_msg_to_synth(&learning_sequencer->note[learning_sequencer->last_index]); 		//send msg to synth

	if((incoming_note_buffer.status >> 4) == 0x9)										//noteOn
	{
		learning_sequencer->last_index++;												//increment learning_sequencer last index
		learn_seq_trigger_toggles +=2;
	}
	else if ((incoming_note_buffer.status >> 4) == 0x8)									//noteOff
	{
		if(gate_interval > (TOGGLETIME >> 1)) 											//separated
		{
			learning_sequencer->last_index++;											//increment learning_sequencer last index
			learn_seq_trigger_toggles +=2;
		}
		else																			//not separated - trigger
		{
			if(incoming_note_buffer.data1 == learning_sequencer->note[learning_sequencer->last_index - 1].data1
					&& (learning_sequencer->note[learning_sequencer->last_index - 1].status >> 4) == 0x9) //trigger case
			{
				//we get noteOff close to noteOn and call it trigger
				//should delete noteOff msg from learning_sequencer

				midi_msg_init(&learning_sequencer->note[learning_sequencer->last_index]); //delete noteOff from learning_sequencer
				gate_interval = 0;
				learning_sequencer->note[learning_sequencer->last_index - 1].trig = 1; 	//mark previous noteOn msg as trigger
																						//and NOT increase last index
			}
			else
			{
				//noteOff after noteOff or noteOff after other note noteOn
				learning_sequencer->last_index++;
				learn_seq_trigger_toggles +=2;
			}
		}
	}
	if(learning_sequencer->last_index == STEPSINSEQ)
	{
		if((learning_sequencer->note[learning_sequencer->last_index - 1].status >> 4) == 0x9 && learning_sequencer->note[learning_sequencer->last_index - 1].trig == 0)
		{
			learning_sequencer->note[learning_sequencer->last_index - 1].trig = 1;
			send_noteOff_for_noteOn(learning_sequencer->note[learning_sequencer->last_index - 1]);
		}
		sequencer_end_of_learn(learning_sequencer);
	}


/*
	learning_sequencer->note[learning_sequencer->last_index] = incoming_note_buffer;
	if(gate_interval > (TOGGLETIME >> 1) || gate_interval == 0)
	{
		//we get noteOff separated from noteOn
		//or we get noteOn
		midi_msg_to_synth(&learning_sequencer->note[learning_sequencer->last_index++]); //send msg to synth and increment learning_sequencer last index

		//word_to_serial("+nON ");
		//midi_msg_print(&learning_sequencer->note[learning_sequencer->last_index]);      //debug
	}
	else
	{
		//we get noteOff close to noteOn and call it trigger
		//should delete noteOff msg from learning_sequencer

		midi_msg_init(&learning_sequencer->note[learning_sequencer->last_index]); //delete noteOff from learning_sequencer
		gate_interval = 0;
		learning_sequencer->note[learning_sequencer->last_index - 1].trig = 1; //mark previous noteOn msg as trigger and NOT increase last index

		word_to_serial("+trig ");
		midi_msg_to_synth(&learning_sequencer->note[learning_sequencer->last_index]); //send to synth to shut down sound
	}
	if(learning_sequencer->last_index == STEPSINSEQ)
		//Need special check. If last msg is noteOn it MUST be trigger
		sequencer_end_of_learn(learning_sequencer);
*/
}

void pause_append_to_sequencer(struct midi_container *learning_sequencer)
{
	learning_sequencer->note[learning_sequencer->last_index++].trig = 1;
	learn_seq_trigger_toggles +=2;

	//word_to_serial("+ pause\r"); //debug

	if(learning_sequencer->last_index == STEPSINSEQ)
		//Need special check. If last msg is noteOn it MUST be trigger
		sequencer_end_of_learn(learning_sequencer);
}

