/*
 * learn_mode.c
 *
 *  Created on: 3 ���. 2021 �.
 *      Author: Vadim
 *
 *  Comment:
 *  - All about Learn, Prev and Next Buttons
 */
#include "main.h"
#include "functions.h"

struct midi_container 		learning_sequencer_buffer;
struct note_stack_element 	learning_stelement_buffer;
struct poly_stack			learning_plelement_buffer;
struct note_range 			learning_mtelement_buffer;
struct note_trans 			learning_trelement_buffer;

extern uint8_t				border_flag;
extern struct led learn_led;
extern uint8_t delete_complete_flag;
extern uint8_t sweep_flag;
extern uint8_t working_mode;

extern uint8_t active_sequencer_index;



extern struct midi_msg trigger_list[WAITHEIGHT];					//array of trigger notes.
extern uint8_t 		trigger_push_index;
extern uint8_t 		trigger_pop_index;

/////////////
////SEQUENCER
void sequncer_start_of_learn(struct midi_container *active_sequencer)
{
	forced_note_off(active_sequencer);
	midi_container_init(&learning_sequencer_buffer);
	learning_sequencer_buffer = *active_sequencer;
	midi_container_init(active_sequencer);

	active_sequencer->learn = 1; //this must be after init
	working_mode = LEARNMODE;

}

void sequencer_end_of_learn(struct midi_container *active_sequencer)
{
	active_sequencer->learn = 0;
	working_mode = PLAYMODE;
	if(active_sequencer->last_index == 0 && delete_complete_flag == 0) //nothing changed and not deleted just init of sequencer
	{
		*active_sequencer = learning_sequencer_buffer;
		 active_sequencer -> active_notes = 0;
		 active_sequencer -> pop_index = 0;
		 active_sequencer -> trigger_toggles = 0;
	}
	/*
	else if(delete_complete_flag == 1)
	{
		//do nothing because sequencer's been already initialized
	}
	*/
	else 												//new data saved
		sweep_flag = 1;

	index_show_set();
	delete_procedure_reset();
}

void forced_note_off(struct midi_container *active_sequencer) //send noteOff message for every noteOn message in active sequencer
{
	for(int i = 0; i < STEPSINSEQ; i++)
		if((active_sequencer->note[i].status >> 4) == 9)
			send_noteOff_for_noteOn(active_sequencer->note[i]);
}

void real_next_sequencer(struct midi_container *active_sequencer)
{
	if(working_mode == PLAYMODE)
	{
		active_sequencer_index++;
		active_sequencer_index %= SEQAMOUNT;
		index_show_set();
	}
	number_to_serial_human_readable(active_sequencer_index);//debug
	word_to_serial("\r");									//debug
}

void real_prev_sequencer(struct midi_container *active_sequencer)
{
	if(working_mode == PLAYMODE)
	{
		if(active_sequencer_index == 0)
			active_sequencer_index = SEQAMOUNT;
		active_sequencer_index--;
		index_show_set();
	}
	number_to_serial_human_readable(active_sequencer_index);//debug
	word_to_serial("\r");									//debug
}
///////////
////STACK
void stack_start_of_learn(struct note_stack_element *active_stelement) //to stack_control_buttons.c
{
	if(active_stelement->active != 0)
	{
		send_noteOff_for_noteOn(active_stelement->stack_note_left);
		// Если во время захода в лерн, ячейка не пустая, то нужно отправить нотОф по адресу
		// Однако потом в системе появится лишний нотОф с клавиатуры. Поэтому в лерн не стоит заходить находу
	}
	delete_notes_from_stack_element(active_stelement);

	note_stack_element_init(&learning_stelement_buffer);
	learning_stelement_buffer = *active_stelement; //copy Dest and Open

	note_stack_element_init(active_stelement);
	active_stelement->learn = 1;
	working_mode = LEARNMODE;
}
void stack_end_of_learn(struct note_stack_element *active_stelement) //to stack_control_buttons.c
{
	active_stelement->learn = 0;
	working_mode = PLAYMODE;
	if((active_stelement->en == 0) && (delete_complete_flag == 0))
	{
		*active_stelement = learning_stelement_buffer;
		index_show_set();
	}
	else
		sweep_flag = 1;

	delete_procedure_reset();
}

void real_prev_stelement(struct note_stack_element *active_stelement, uint8_t *stindex)
{
	if(working_mode == PLAYMODE)
	{
		if(*stindex == 0)
			*stindex = STACKHEIGHT;
		*stindex -= 1;
		index_show_set();
	}

	number_to_serial_human_readable(*stindex);	//debug
	word_to_serial("\r");						//debug
}

void real_next_stelement(struct note_stack_element *active_stelement, uint8_t *stindex)
{
	if(working_mode == PLAYMODE)//���� �� ����� �������, ������ ��� � ���������� ��� ���� ��������
	{
		*stindex += 1;
		*stindex %= STACKHEIGHT;
		index_show_set();
	}

	number_to_serial_human_readable(*stindex); 	//debug
	word_to_serial("\r");						//debug
}


/////////
////MULT
void real_prev_mtelement(struct note_range_array *range_array)
{
	if(range_array->active_mtindex == 0)
		range_array->active_mtindex = MULTTIMBREAMOUNT;
	range_array->active_mtindex--;
	index_show_set();

//	number_to_serial_human_readable(range_array->active_mtindex); 		//debug
//	word_to_serial(" pl\r");											//debug

}
void real_next_mtelement(struct note_range_array *range_array)
{
	range_array->active_mtindex++;
	range_array->active_mtindex %= MULTTIMBREAMOUNT;
	index_show_set();

//	number_to_serial_human_readable(range_array->active_mtindex); 		//debug
//	word_to_serial(" pl\r");											//debug
}
void real_next_mtparam(struct note_range *range)
{
	range->active_mtparamindex++;
	range->active_mtparamindex%=MULTPARAMAMOUNT;

	number_to_serial_human_readable(range->active_mtparamindex); 		//debug
	word_to_serial(" lr\r");											//debug
}
void real_prev_mtparam(struct note_range *range)
{
	if(range->active_mtparamindex == 0)
		range->active_mtparamindex = MULTPARAMAMOUNT;
	range->active_mtparamindex--;

//	number_to_serial_human_readable(range->active_mtparamindex); 		//debug
//	word_to_serial(" lr\r");											//debug
}
void real_start_range_note_trigger(struct note_range *range)
{
	struct midi_msg start_trig;
	midi_msg_init(&start_trig);

	start_trig.status = (0x9 << 4) + range->destination;
	start_trig.data1 = range->parameters[MULTSTARTPos];
	start_trig.data2 = 64;

	// Change note octave
	start_trig.data1 = mult_note_transpose(range, start_trig.data1);

//	uint8_t temp_shift = range->parameters[MULTRANGESHIFTPos];
//	if(temp_shift == 0) //collapse to low
//	{
//		start_trig.data1 = range->parameters[MULTSTARTPos];
//	}
//	else if(temp_shift == 7)// collapse to high
//	{
//		start_trig.data1 = range->parameters[MULTENDPos];;
//	}
//	else if(temp_shift == 15) //inversion
//	{
//		start_trig.data1 = 127 - start_trig.data1;
//	}
//	else if(temp_shift == 14) // stretch
//	{
//		uint8_t note_start = range->parameters[MULTSTARTPos];
//		uint8_t note_end   = range->parameters[MULTENDPos];
//		start_trig.data1 =  127*(start_trig.data1 - note_start)/(note_end - note_start);
//	}
//	else if(temp_shift < 7)
//	{
//		while(temp_shift > 0 && start_trig.data1 > 12)
//		{
//			start_trig.data1 -= 12;
//			temp_shift--;
//		}
//	}
//	else if(temp_shift > 8)
//	{
//		while(temp_shift > 8 && start_trig.data1 < 115)
//		{
//			start_trig.data1 += 12;
//			temp_shift--;
//		}
//	}


	/// это надо оформить во внятную функцию
	trigger_list[trigger_push_index++] = start_trig;
	midi_msg_to_synth(&start_trig);
	trigger_noteOff_counter_set();
}
void real_end_range_note_trigger(struct note_range *range)
{
	struct midi_msg end_trig;
	midi_msg_init(&end_trig);
	end_trig.status = (0x9 << 4) + range->destination;
	end_trig.data1 = range->parameters[MULTENDPos];
	end_trig.data2 = 64;

	// Change note octave
	end_trig.data1 = mult_note_transpose(range, end_trig.data1);
//	uint8_t temp_shift = range->parameters[MULTRANGESHIFTPos];
//	if(temp_shift == 0) //collapse to low
//	{
//		end_trig.data1 = range->parameters[MULTSTARTPos];
//	}
//	else if(temp_shift == 7)// collapse to high
//	{
//		end_trig.data1 = range->parameters[MULTENDPos];;
//	}
//	else if(temp_shift == 15) //inversion
//	{
//		end_trig.data1 = 127 - end_trig.data1;
//	}
//	else if(temp_shift == 14) // stretch
//	{
//		uint8_t note_start = range->parameters[MULTSTARTPos];
//		uint8_t note_end   = range->parameters[MULTENDPos];
//		end_trig.data1 =  127*(end_trig.data1 - note_start)/(note_end - note_start);
//	}
//	else if(temp_shift < 7)
//	{
//		while(temp_shift > 0 && end_trig.data1 > 12)
//		{
//			end_trig.data1 -= 12;
//			temp_shift--;
//		}
//	}
//	else if(temp_shift > 8)
//	{
//		while(temp_shift > 8 && end_trig.data1 < 115)
//		{
//			end_trig.data1 += 12;
//			temp_shift--;
//		}
//	}

	/// это надо оформить во внятную функцию
	trigger_list[trigger_push_index++] = end_trig;
	midi_msg_to_synth(&end_trig);
	trigger_noteOff_counter_set();
}


void mult_start_of_learn(struct note_range *active_mtelement)
{
	if(active_mtelement->active != 0)
	{
		// в элементе есть неотжатые ноты. Либо забиваем, либо не пускаем в лёрн
		// Нужно прописать вариант с непусканием в лёрн, если прокатит, то повторю для поли и транса
	}

	note_range_init(&learning_mtelement_buffer);
	learning_mtelement_buffer = *active_mtelement; //copy Dest and Open

	active_mtelement->learn = 1;
	working_mode = LEARNMODE;

}
void mult_end_of_learn(struct note_range *active_mtelement)
{
	active_mtelement->learn = 0;
	working_mode = PLAYMODE;
	if((active_mtelement->en == 0) && (delete_complete_flag == 0)) // что значит это условие?
	{
		//*active_mtelement = learning_mtelement_buffer;
		//active_mtelement->en = 1;
		//index_show_set();
	}
	else if(delete_complete_flag == 1)
	{
		note_range_init(active_mtelement);
		active_mtelement->en = 0;
	}
	else
		sweep_flag = 1;
	border_flag = 0;
	mult_borders_swap();

	delete_procedure_reset();
	index_show_set();

}



//////////
////TRANS
void real_prev_trelement(struct note_trans_array *trans_array)
{
	if(trans_array->active_trindex == 0)
		trans_array->active_trindex = TRANSCCAMOUNT;
	trans_array->active_trindex--;
	index_show_set();

	number_to_serial_human_readable(trans_array->active_trindex); 	//debug
	word_to_serial(" pl\r");										//debug

}
void real_next_trelement(struct note_trans_array *trans_array)
{
	trans_array->active_trindex++;
	trans_array->active_trindex %= TRANSCCAMOUNT;
	index_show_set();

	number_to_serial_human_readable(trans_array->active_trindex); 	//debug
	word_to_serial(" pl\r");											//debug
}
void real_next_trparam(struct note_trans *translator)
{
	translator->active_trparamindex++;
	translator->active_trparamindex %= TRANSPARAMAMOUNT;

	number_to_serial_human_readable(translator->active_trparamindex); 	//debug
	word_to_serial(" lr trans\r");											//debug
}
void real_prev_trparam(struct note_trans *translator)
{
	if(translator->active_trparamindex == 0)
		translator->active_trparamindex = TRANSPARAMAMOUNT;
	translator->active_trparamindex--;

	number_to_serial_human_readable(translator->active_trparamindex); 	//debug
	word_to_serial(" lr trans\r");
}


void trans_start_of_learn(struct note_trans *active_trelement)
{
	if(active_trelement->active != 0)
	{
		// � ������ ���� ��������� ����. ���� ��������, ���� �� ������� � ���
	}

	note_trans_init(&learning_trelement_buffer);
	learning_trelement_buffer = *active_trelement;

	//note_range_init(active_mtelement);

	active_trelement->learn = 1;
	///active_trelement->en = 0;
	working_mode = LEARNMODE;
	word_to_serial("enter learn\r"); 					//debug
}
void trans_end_of_learn(struct note_trans *active_trelement)
{
	if((active_trelement->en == 0) && (delete_complete_flag == 0))
	{
		//*active_mtelement = learning_mtelement_buffer;
		///active_trelement->en = 1;
		//index_show_set();
	}
	else if(delete_complete_flag == 1)
	{
		note_trans_init(active_trelement);
	}
	else
		sweep_flag = 1;

	active_trelement->learn = 0;
	working_mode = PLAYMODE;
	word_to_serial("exit learn\r");

	delete_procedure_reset();
	index_show_set();


}
