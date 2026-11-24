/*
 * poly_mode_actions.c
 *
 *  Created on: 4 ���. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

void push_incoming_note_to_stack(struct note_stack *stack, struct midi_msg push_note)
{
	int push_index;
	if	   ((push_note.status >> 4) == NOTEON)
	{
		push_index = find_pair_index(stack, push_note); 		//first try to find pair in stack
		if(push_index == -1)
			push_index = find_free_slot_index(stack, push_note);

		push_note_to_stack_element(&(stack->stelement[push_index]), push_note);
		if(push_index < STACKHEIGHT)
		{
			midi_msg_to_synth(&(stack->stelement[push_index].stack_note_left));

//			word_to_serial("+ "); //debug
//			midi_msg_print(&(stack->stelement[push_index].stack_note_left));//debug
		}
	}
	else if((push_note.status >> 4) == NOTEOFF)
	{
		push_index = find_pair_index(stack, push_note);
		if    (push_index == -1)
		{
//			word_to_serial("%\r");
			push_index = find_free_slot_index(stack, push_note);
			push_note_to_stack_element(&(stack->stelement[push_index]), push_note);
		}
		else //there is pair
		{
			push_note_to_stack_element(&(stack->stelement[push_index]), push_note);
			if(push_index >= STACKHEIGHT)//pair in waiting part of stack
			{
				delete_notes_from_stack_element(&(stack->stelement[push_index]));

//				word_to_serial("xx \r"); //debug
			}
			else if(push_index < STACKHEIGHT)//pair in sounding part
			{
				midi_msg_to_synth(&(stack->stelement[push_index].stack_note_right));

//				word_to_serial("- "); //debug
//				midi_msg_print(&(stack->stelement[push_index].stack_note_right));//debug

				delete_notes_from_stack_element(&(stack->stelement[push_index]));
				///������ ������, ���� �������� ������
				if(stack->stelement[push_index].open == 1)
					move_noteOn_from_waiting_to_sounding_stack_element(stack, &(stack->stelement[push_index]));
			}
		}
	}
}
void move_noteOn_from_waiting_to_sounding_stack_element(struct note_stack *stack, struct note_stack_element *dest_stelement)
{
	int move_index;
	move_index = find_waiting_noteOn_index(stack, STACKHEIGHT);
	if(move_index != -1)//there is waiting noteOn
	{
		push_note_to_stack_element(dest_stelement,stack->stelement[move_index].stack_note_left);

		delete_notes_from_stack_element(&(stack->stelement[move_index]));

//		number_to_serial_human_readable(move_index);//debug
//		word_to_serial("x \r");//debug

		dest_stelement->moving_toggles += 2;

		midi_msg_to_synth(&(dest_stelement->stack_note_left));

//		word_to_serial("+ ");//debug
//		midi_msg_print(&(dest_stelement->stack_note_left));//debug
	}
}

void push_note_to_stack_element(struct note_stack_element *stack_element, struct midi_msg push_note)
{
	if     ((push_note.status >> 4) == 9)//noteOn
	{
		stack_element->stack_note_left = push_note;
		stack_element->stack_note_left.status &= 0xF0;
		stack_element->stack_note_left.status += stack_element->destination;
		stack_element->active++;
	}
	else if((push_note.status >> 4) == 8)//noteOff
	{
		stack_element->stack_note_right = push_note;
		stack_element->stack_note_right.status &= 0xF0;
		stack_element->stack_note_right.status += stack_element->destination;
		stack_element->active++;
	}
}
void delete_notes_from_stack_element(struct note_stack_element *stack_element)
{
	midi_msg_init(&(stack_element->stack_note_left));
	midi_msg_init(&(stack_element->stack_note_right));
	stack_element->active = 0;
}

int  find_pair_index(struct note_stack *stack, struct midi_msg push_note)
{
	if	   ((push_note.status >> 4) == NOTEON)
	{
		for(int i = 0; i < WAITHEIGHT; i++)
		{
			if(stack->stelement[i].active != 0 && stack->stelement[i].stack_note_right.data1 == push_note.data1)
				return i;
		}
	}
	else if((push_note.status >> 4) == NOTEOFF)
	{
		for(int i = 0; i < WAITHEIGHT; i++)
		{
			if(stack->stelement[i].active != 0 && stack->stelement[i].stack_note_left.data1 == push_note.data1)
				return i;
		}
	}
	return -1;
}
int  find_free_slot_index(struct note_stack *stack, struct midi_msg push_note) //first free slot!
{
	if	   ((push_note.status >> 4) == NOTEON)
	{
		uint8_t start_ind = stack->active_stindex;
		for(int i = start_ind; i < STACKHEIGHT; i++)
		{
			if(stack->stelement[i].active == 0 && stack->stelement[i].en == 1 && stack->stelement[i].open == 1)
				return i;
		}
		for(int i = 0; i < start_ind; i++)
		{
			if(stack->stelement[i].active == 0 && stack->stelement[i].en == 1 && stack->stelement[i].open == 1)
				return i;
		}
		/*
		for(int i = 0; i < STACKHEIGHT; i++)
		{
			if(stack->stelement[i].active == 0 && stack->stelement[i].en == 1 && stack->stelement[i].open == 1)
				return i;
		}
		*/
	}
	for(int i = STACKHEIGHT; i < WAITHEIGHT; i++) //same for noteOn and noteOff
	{
		if(stack->stelement[i].active == 0)
			return i;
	}
	return WAITHEIGHT - 1;

}
int  find_waiting_noteOn_index(struct note_stack *stack, int start_index)
{
	for(int i = STACKHEIGHT; i < WAITHEIGHT; i++)
	{
		if(stack->stelement[i].active == 1 && stack->stelement[i].stack_note_left.status != 0)
			return i;
	}
	return -1;
}
