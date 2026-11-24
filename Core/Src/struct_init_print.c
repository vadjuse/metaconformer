/*
 * midi_msg_functions.c
 *
 *  Created on: 23 ����. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

extern int midi_access_semaph;

//////////
////MIDI
void send_noteOff_for_noteOn(struct midi_msg tnote)//change status byte from 0x9n to 0x8n and send
{
	struct midi_msg temp_noteOff = tnote;
	if((temp_noteOff.status >> 4) == 0x9)
	{
		//for(int i = 0; i < 4000; i++);
		temp_noteOff.status -= 0x10; //0x8n n is for channel number
		temp_noteOff.data2  = 0;
	}
	//midi_msg_print(&temp_noteOff); //debug
	//while(midi_access_semaph == 1);
	midi_msg_to_synth(&temp_noteOff);

}
void midi_msg_print(struct midi_msg *note)
{

//	word_to_serial(" 0x");
//	number_to_serial_human_readable(note->status);
//	number_to_serial_human_readable(note->data1);
//	number_to_serial_human_readable(note->data2);
//	number_to_serial_human_readable(note->trig);
//	byte_to_serial('\r');
}

void midi_msg_to_synth(struct midi_msg *note)
{
	midi_access_semaph = 1;
	byte_to_midi_device(note->status);
	byte_to_midi_device(note->data1);
	byte_to_midi_device(note->data2);

	// DEBUG
	//number_to_serial_human_readable(note->status);
	//number_to_serial_human_readable(note->data1);
	//number_to_serial_human_readable(note->data2);
	//byte_to_serial('\r');
	midi_access_semaph = 0;
}

void midi_chanafttouch_to_synth(struct midi_msg *note)
{
	byte_to_midi_device(note->status);
	byte_to_midi_device(note->data1);
}

void midi_msg_init(struct midi_msg *note)
{
	note->status = 0;
	note->data1 = 0;
	note->data2 = 0;
	note->complete_flag = 0;
	note->trig = 0;
}
//////////////
////SEQUENCER
void midi_container_init(struct midi_container *sequencer)
{
	sequencer->last_index = 0;
	sequencer->temp_index = 0;
	sequencer->pop_index = 0;
	sequencer->learn = 0;
	sequencer->active_notes = 0;
	sequencer->trigger_toggles = 0;
	for(int i = 0; i < STEPSINSEQ; i++)midi_msg_init(&(sequencer->note[i]));
}

void sequencer_array_init(struct midi_container *sequencer)
{
	for(int i = 0; i < SEQAMOUNT; i++)
			midi_container_init(sequencer+i);
}

void midi_container_print(struct midi_container *sequencer, uint8_t num)
{
	number_to_serial_human_readable(num);
	word_to_serial(" seq: \r");
	for(int i = 0; i < STEPSINSEQ; i++)midi_msg_print(&(sequencer->note[i]));
	word_to_serial(" \r");
	word_to_serial(" active notes: ");
	number_to_serial_human_readable(sequencer->active_notes);
	word_to_serial(" \r");

}
/////////
////STACK
void note_stack_element_init(struct note_stack_element *stelement)
{
	stelement->open = 1;
	stelement->active = 0;
	stelement->en = 0;
	stelement->destination = 0;
	stelement->learn = 0;
	stelement->moving_toggles = 0;
	midi_msg_init(&(stelement->stack_note_left));
	midi_msg_init(&(stelement->stack_note_right));
}

void note_stack_element_print(struct note_stack_element *stelement)
{
	/*
	Word_To_Serial("st: ");
	Number_To_Serial(stelement->status_register);
	*/
	word_to_serial("ac:en:op: ");
	number_to_serial_human_readable(stelement->active);
	number_to_serial_human_readable(stelement->en);
	number_to_serial_human_readable(stelement->open);
	word_to_serial(" dst: 0x");
	number_to_serial_human_readable(stelement->destination);
	// These elements must be cleared while learn mode
	word_to_serial(" msg: ");
	midi_msg_print(&stelement->stack_note_left);
	midi_msg_print(&stelement->stack_note_right);

	//Word_To_Serial(" \r");

}

void note_stack_init(struct note_stack *stack)
{
	stack->max_index = STACKHEIGHT;
	stack->active_stindex = 0;
	stack->input_channel = 0;
	for(int i = 0; i < WAITHEIGHT; i++)
	{
		note_stack_element_init(&stack->stelement[i]);
	}

}

void note_stack_print(struct note_stack *stack)
{
	word_to_serial("Stack:\r");
	word_to_serial("fs: ");
	number_to_serial_human_readable(stack->max_index);
	word_to_serial(" ind: ");
	number_to_serial_human_readable(stack->active_stindex);
	word_to_serial(" in: ");
	number_to_serial_human_readable(stack->input_channel);
	word_to_serial(" \r");
	for(int i = 0; i < WAITHEIGHT; i++)
		note_stack_element_print(&stack->stelement[i]);
}


void poly_stack_init(struct poly_stack *plelement)
{
	plelement->open = 1;
	plelement->active = 0;
	plelement->en = 0;
	plelement->destination = 0;
	plelement->learn = 0;
	plelement->moving_toggles = 0;

	plelement->active_plindex = 0;
	for(int i = 0; i < WAITHEIGHT; i++)
		midi_msg_init(&(plelement->poly_note_array[i]));

}
void poly_stack_print(struct poly_stack *plelement)
{
	word_to_serial("ac:en:op: ");
	number_to_serial_human_readable(plelement->active);
	number_to_serial_human_readable(plelement->en);
	number_to_serial_human_readable(plelement->open);
	word_to_serial(" dst: 0x");
	number_to_serial_human_readable(plelement->destination);
	// These elements must be cleared while learn mode
	word_to_serial(" last: ");
	midi_msg_print(&(plelement->poly_note_array[plelement->active_plindex]));
}

void poly_stack_array_init(struct poly_stack_array *plarray)
{
	plarray->input_channel = 0;
	plarray->last_put_index= 0;
	for(int i = 0; i < STACKHEIGHT; i++)
		poly_stack_init(&plarray->plelement[i]);
}
void poly_stack_array_print(struct poly_stack_array *plarray)
{
	word_to_serial("Poly array:\r");
	word_to_serial("inch: ");
	number_to_serial_human_readable(plarray->input_channel);
	word_to_serial(" last put: ");
	number_to_serial_human_readable(plarray->last_put_index);
	word_to_serial(" \r");
	for(int i = 0; i < STACKHEIGHT; i++)
		poly_stack_print(&plarray->plelement[i]);
}
///////////
////MULT
void note_range_array_init(struct note_range_array *range_array)
{
	range_array->max_index = MULTTIMBREAMOUNT;
	range_array->active_mtindex = 0;
	range_array->input_channel = 0;
	for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		note_range_init(&range_array->mtelement[i]);
}

void note_range_init(struct note_range *note_range)
{
	note_range->active_mtparamindex = 0;
	//for(int i = 0; i < MULTYPARAMAMOUNT; i++)
	//	note_range->parameters[i] = 128;
	note_range->parameters[MULTRANGESHIFTPos] = 8;
	note_range->parameters[MULTSTARTPos] = 0;
	note_range->parameters[MULTENDPos] = 0;

	note_range->destination = 0;
	note_range->active = 0;
	note_range->moving_toggles = 0;
	note_range->en = 0;
	note_range->learn = 0;
	note_range->open = 1;
}

void note_range_print(struct note_range *note_range)
{
	word_to_serial("outch: ");
	number_to_serial_human_readable(note_range->destination);
	word_to_serial(" start: ");
	number_to_serial_human_readable(note_range->parameters[MULTSTARTPos]);
	word_to_serial(" end: ");
	number_to_serial_human_readable(note_range->parameters[MULTENDPos]);
	word_to_serial(" \r");
}
void note_range_array_print(struct note_range_array *range_array)
{
	word_to_serial("range array: \r inch: ");
	number_to_serial_human_readable(range_array->input_channel);
	word_to_serial("\r");
	for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		note_range_print(&range_array->mtelement[i]);
}
///////////
////TRANS
void note_trans_print(struct note_trans *note_trans);
void note_trans_array_print(struct note_trans_array *trans_array);

void note_trans_array_init(struct note_trans_array *trans_array)
{
	trans_array->max_index = TRANSCCAMOUNT;
	trans_array->active_trindex = 0;
//	trans_array->input_channel = 0;
	for(int i = 0; i < TRANSCCAMOUNT; i++)
		note_trans_init(&trans_array->trelement[i]);
}

void note_trans_init(struct note_trans *note_trans)
{
	note_trans->active_trparamindex = 0;
	note_trans->parameters[TRANSCCPos] = 0;
	note_trans->parameters[TRANSSRCPos] = 0;
	note_trans->parameters[TRANSDSTPos] = 0;

	note_trans->destination = 0;
	note_trans->active = 0;
	note_trans->moving_toggles = 0;
	note_trans->en = 0;
	note_trans->learn = 0;
	note_trans->open = 1;
}

void note_trans_print(struct note_trans *note_trans)
{
	word_to_serial(" CC: ");
	number_to_serial_human_readable(note_trans->parameters[TRANSCCPos]);
	word_to_serial(" src: ");
	number_to_serial_human_readable(note_trans->parameters[TRANSSRCPos]);
	word_to_serial(" dst: ");
	number_to_serial_human_readable(note_trans->parameters[TRANSDSTPos]);
	word_to_serial(" \r");
}
void note_trans_array_print(struct note_trans_array *trans_array)
{
	word_to_serial("translator array: \r");
	//number_to_serial_human_readable(range_array->input_channel);
	//word_to_serial("\r");
	for(int i = 0; i < TRANSCCAMOUNT; i++)
		note_trans_print(&(trans_array->trelement[i]));
}
///////////
////CONFIG
void cfg_init(struct config_arrays *config)
{
	for(int i = 0; i < SEQAMOUNT; i++)
		config->seq[i] = 0;
	for(int i = 0; i < STACKHEIGHT; i++)
		config->stack[i] = 0;
	for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		config->mult[i] = 0;
	for(int i = 0; i < TRANSCCAMOUNT; i++)
		config->trans[i] = 0;

	config->seq[MIDICLKPOLPos] = 0;
	config->seq[MIDICLKPSCPos] = 7;
//	config->seq[MIDIOUT2COPYTHRUPos] = 0xA;
//	config->seq[6] = 0x7;
	config->seq[5] = 0x3;

	config->stack[POLYINCHPos] = 0;
//	config->stack[POLYVELOBIASPos] = 7;
//	config->stack[MIDIOUT2COPYTHRUPos] = 15;

	config->trans[POLYINCHPos] = 2;
//	config->trans[POLYVELOBIASPos] = 9;
//	config->trans[MIDIOUT2COPYTHRUPos] = 14;

	config->active_index = 0;
	config->max_index_seq = 8;
	config->max_index_poly = 8;
	config->max_index_mult = 8;
	config->max_index_trans = 8;
}



