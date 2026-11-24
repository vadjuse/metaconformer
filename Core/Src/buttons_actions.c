/*
 * buttons_actions.c
 *
 *  Created on: 27 ���. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"


extern struct midi_msg trigger_list[WAITHEIGHT];					//array of trigger notes.
extern uint8_t 		   trigger_push_index;
extern uint8_t 		   trigger_pop_index;

extern struct midi_msg seq_msg_list[WAITHEIGHT];					//array of sequencer messages need to be sensded.
extern uint8_t 		   seq_msg_push_index;
extern uint8_t 		   seq_msg_pop_index;
extern int			   midi_access_semaph;
 int 			seq_list_flag;


/////////////
////SEQUENCER
void real_pop_from_sequencer(struct midi_container *pop_sequencer, uint8_t index)
{
	if(pop_sequencer->last_index != 0 && pop_sequencer->learn == 0)
	{
		//while(midi_access_semaph == 1);
		midi_msg_to_synth(&pop_sequencer->note[pop_sequencer->pop_index]);

		/*
		seq_list_flag = 1;
		seq_msg_list[seq_msg_push_index] = (pop_sequencer->note[pop_sequencer->pop_index]);
		seq_msg_push_index++;
		seq_list_flag = 0;
		*/


//		word_to_serial("*"); 													//debug
//		number_to_serial_human_readable(seq_msg_push_index);									//debug
//		midi_msg_print(&pop_sequencer->note[pop_sequencer->pop_index]);			//debug


		if(pop_sequencer->note[pop_sequencer->pop_index].trig == 0) 	//not trigger
		{
			if((pop_sequencer->note[pop_sequencer->pop_index].status >> 4) == 9 && pop_sequencer->active_notes < 20)
				pop_sequencer->active_notes++;
			else if((pop_sequencer->note[pop_sequencer->pop_index].status >> 4) == 8 && pop_sequencer->active_notes > 0)
				pop_sequencer->active_notes--;
		}
		else //trigger
		{
			//send_noteOff_for_noteOn(pop_sequencer->note[pop_sequencer->pop_index]);//��� ��������� ������ ���� ���������� � ��������� ����� noteOn
			if(pop_sequencer->trigger_toggles < 21)
				pop_sequencer->trigger_toggles += 2;
			trigger_list[trigger_push_index++] = (pop_sequencer->note[pop_sequencer->pop_index]);
			trigger_noteOff_counter_set();
		}

		pop_sequencer->pop_index++;
		pop_sequencer->pop_index %= pop_sequencer->last_index;
	}
}
//////////
////STACK
void real_stelement_mute_toggle(struct note_stack *m_stack, struct note_stack_element *pressed_stelement ,uint8_t index)
{
	// убрать real в начале, а в другой функции добавить приставку wrapper
	if(pressed_stelement->learn == 0 && pressed_stelement->en == 1)
	{
		pressed_stelement->open = (pressed_stelement->open == 0) ? 1 : 0; //toggle open flag
		if(pressed_stelement->open == 0)
		{
			//do nothing
		}
		else if(pressed_stelement->open == 1)
		{
			//if opened element is empty put waiting noteOn into if there is one
			if(pressed_stelement->active == 0)
				move_noteOn_from_waiting_to_sounding_stack_element(m_stack,pressed_stelement);
		}
		show_mutes_set();
	}
}

/////////
////MULT
void real_mtelement_mute_toggle(struct note_range_array *range, struct note_range *pressed_mtelement ,uint8_t index)
{
	// убрать real в начале, а в другой функции добавить приставку wrapper
	if(pressed_mtelement->learn == 0 && pressed_mtelement->en == 1)
	{
		pressed_mtelement->open = (pressed_mtelement->open == 0) ? 1 : 0; //toggle open flag
		show_mutes_set();
	}
}

/////////
////TRANS
void real_trelement_mute_toggle(struct note_trans_array *trans_array, struct note_trans *pressed_trelement ,uint8_t index)
{
	if(pressed_trelement->learn == 0 && pressed_trelement->en == 1)
	{
		pressed_trelement->open = (pressed_trelement->open == 0) ? 1 : 0; //toggle open flag

		show_mutes_set();
	}
}

