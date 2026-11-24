/*
 * irqs.c
 *
 *  Created on: 23 ����. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

#define DEBOUNCE 1200

extern uint8_t working_mode;
extern uint8_t index_show_flag;
extern struct config_arrays main_cfg;

extern struct led learn_led;
extern uint8_t global_mode;

extern struct note_stack main_stack;
extern struct note_range_array main_mult;

uint8_t previous_button_state_register = 0xFF;
uint8_t current_button_state_register  = 0xFF;


uint8_t  tx_usart1_queue[1024];
int      tx_usart1_index = 0;
int 	 tx_usart1_end 	= 0;
int 	 tx_usart1_busy = 0;

uint8_t  tx_usart4_queue[1024];
int      tx_usart4_index = 0;
int 	 tx_usart4_busy = 0;
int 	 tx_usart4_end 	= 0;

void EXTI15_10_IRQHandler()
{
	if((EXTI->PR & (1 << PREVBTNPIN)) != 0)// 12 line interrupt occur
	{
		EXTI->PR |= 1 << PREVBTNPIN; //clear pending bit
		for(int i = 0; i < DEBOUNCE; i++); //debounce delay
		if(global_mode == SEQMODE)
		{
			if(working_mode != CFGMODE)
			{
				prev_sequencer();
			}
			else
			{
				if(main_cfg.active_index == 0)
					main_cfg.active_index = main_cfg.max_index_seq;
				main_cfg.active_index--;

				//debug
//				word_to_serial("active cfg seq mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug end
			}
		}
		else if(global_mode == POLYMODE)
		{
			if(working_mode != CFGMODE)
			{
				prev_stelement();
			}
			else
			{
				if(main_cfg.active_index == 0)
					main_cfg.active_index = main_cfg.max_index_poly;
				main_cfg.active_index--;

				//debug
//				word_to_serial("active cfg poly mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug end

			}
		}
		else if(global_mode == MULTMODE)
		{
			if(working_mode == PLAYMODE)
			{
				prev_mtelement();
			}
			else if(working_mode == CFGMODE)
			{
				if(main_cfg.active_index == 0)
					main_cfg.active_index = main_cfg.max_index_mult;
				main_cfg.active_index--;

				//debug
//				word_to_serial("active cfg mult mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug end
			}
			else if(working_mode == LEARNMODE)
			{
				//prev_mtparam();
				start_range_note_trigger();
			}
		}
		else if(global_mode == TRANSMODE)
		{
			if(working_mode == PLAYMODE)
			{
				prev_trelement();
			}
			else if(working_mode == CFGMODE)
			{
				if(main_cfg.active_index == 0)
					main_cfg.active_index = main_cfg.max_index_trans;
				main_cfg.active_index--;

				//debug
//				word_to_serial("active cfg trans mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug end
			}
			else if(working_mode == LEARNMODE)
			{
				prev_trparam();
			}
		}

	}
	if((EXTI->PR & (1 << CFGBTNPIN)) != 0)// 13 line interrupt occur. CFG button
	{
		EXTI->PR |= 1 << CFGBTNPIN; //clear pending bit
		for(int i = 0; i < DEBOUNCE; i++); //debounce delay

		if(global_mode == SEQMODE)
		{
			if(working_mode == PLAYMODE)
			{
				working_mode = CFGMODE;
//				word_to_serial("enter cfg\r");   //debug
			}
			else if(working_mode == CFGMODE)
			{
				working_mode = PLAYMODE;
				// Store cfg arrays to FLASH
				cfg_store_to_at45db();

				index_show_set();
//				word_to_serial("exit cfg\r"); 	 //debug
			}
		}
		else if(global_mode == POLYMODE)
		{
			if(working_mode == PLAYMODE)
			{
				uint8_t flag = 0;
				for(int i = 0; i < STACKHEIGHT; i++)
				{
					if(main_stack.stelement[i].active != 0)
					{
						flag = 1;
					}
				}

				if(flag == 0)
				{
					working_mode = CFGMODE;
				}

				//working_mode = CFGMODE;
				//word_to_serial("enter cfg\r");   //debug
			}
			else if(working_mode == CFGMODE)
			{
				working_mode = PLAYMODE;
				// Store cfg arrays to FLASH
				cfg_store_to_at45db();
				index_show_set();
//				word_to_serial("exit cfg\r"); 	 //debug
			}
		}
		else if(global_mode == MULTMODE)
		{
			if(working_mode == PLAYMODE)
			{
				uint8_t flag = 0;
				for(int i = 0; i < MULTTIMBREAMOUNT; i++)
				{
					if(main_mult.mtelement[i].active != 0)
					{
						flag = 1;
					}
				}
				if(flag == 0)
				{
					working_mode = CFGMODE;
				}


				//working_mode = CFGMODE;
				//word_to_serial("enter cfg\r");   //debug
			}
			else if(working_mode == CFGMODE)
			{
				working_mode = PLAYMODE;
				// Store cfg arrays to FLASH
				cfg_store_to_at45db();
				index_show_set();
//				word_to_serial("exit cfg\r"); 	 //debug
			}
		}
		else if(global_mode == TRANSMODE)
		{
			if(working_mode == PLAYMODE)
			{
				working_mode = CFGMODE;
//				word_to_serial("enter cfg\r");   //debug
			}
			else if(working_mode == CFGMODE)
			{
				working_mode = PLAYMODE;
				// Store cfg arrays to FLASH
				cfg_store_to_at45db();
				index_show_set();
//				word_to_serial("exit cfg\r"); 	 //debug
			}
		}
	}
}

void EXTI9_5_IRQHandler()
{
	if((EXTI->PR & (1 << LEARNBTNPIN)) != 0)//line interrupt occur Learn button
	{
		EXTI->PR |= 1 << LEARNBTNPIN; //clear pending bit
		for(int i = 0; i < DEBOUNCE; i++); //debounce delay
		if(global_mode == SEQMODE && working_mode != CFGMODE)
		{
			sequencer_learn_mode_toggle();
		}
		else if(global_mode == POLYMODE && working_mode != CFGMODE)
		{
			note_stack_learn_mode_toggle();
		}
		else if(global_mode == MULTMODE && working_mode != CFGMODE)
		{
			note_range_learn_mode_toggle();
		}
		else if(global_mode == TRANSMODE && working_mode != CFGMODE)
		{
			note_trans_learn_mode_toggle();
		}
	}
	if((EXTI->PR & (1 << NEXTBTNPIN)) != 0)// 9 line interrupt occur
	{
		EXTI->PR |= 1 << NEXTBTNPIN; //clear pending bit
		for(int i = 0; i < DEBOUNCE; i++); //debounce delay
		if(global_mode == SEQMODE)
		{
			if(working_mode != CFGMODE)
			{
				next_sequencer();
			}
			else
			{
				main_cfg.active_index++;
				if(main_cfg.active_index == main_cfg.max_index_seq)
					main_cfg.active_index = 0;


				//debug
//				word_to_serial("active cfg seq mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug
			}

		}
		else if(global_mode == POLYMODE)
		{
			if(working_mode != CFGMODE)
			{
				next_stelement();
			}
			else
			{
				main_cfg.active_index++;
				if(main_cfg.active_index == main_cfg.max_index_poly)
					main_cfg.active_index = 0;


				//debug
//				word_to_serial("active cfg poly mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug
			}
		}
		else if(global_mode == MULTMODE)
		{
			if(working_mode == PLAYMODE)
			{
				next_mtelement();
			}
			else if(working_mode == CFGMODE)
			{
				main_cfg.active_index++;
				if(main_cfg.active_index == main_cfg.max_index_mult)
					main_cfg.active_index = 0;


				//debug
//				word_to_serial("active cfg mult mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug
			}
			else if(working_mode == LEARNMODE)
			{
				//next_mtparam();
				end_range_note_trigger();
			}
		}
		else if(global_mode == TRANSMODE)
		{
			if(working_mode == PLAYMODE)
			{
				next_trelement();
			}
			else if(working_mode == CFGMODE)
			{
				main_cfg.active_index++;
				if(main_cfg.active_index == main_cfg.max_index_trans)
					main_cfg.active_index = 0;


				//debug
//				word_to_serial("active cfg trans mode: ");
//				number_to_serial_human_readable(main_cfg.active_index);
//				word_to_serial("\r");
				//debug
			}
			else if(working_mode == LEARNMODE)
			{
				next_trparam();
			}
		}
	}
	line_procedure(7);
	line_procedure(6);
	line_procedure(5);
}

void EXTI4_IRQHandler()
{
	line_procedure(4);
}
void EXTI3_IRQHandler()
{
	line_procedure(3);
}
void EXTI2_IRQHandler()
{
	line_procedure(2);
}
void EXTI1_IRQHandler()
{
	line_procedure(1);
}
void EXTI0_IRQHandler()
{
	line_procedure(0);
}

void line_procedure(uint8_t line)
{
	if((EXTI->PR & (1 << line)) != 0)//line interrupt occur
	{
		EXTI->PR |= 1 << line; //clear pending bit
		for(int i = 0; i < DEBOUNCE; i++); //debounce delay

		current_button_state_register = 0;
		current_button_state_register = GPIOC->IDR & 0xFF; //read [0:7] bits of register
		for(int i = 0; i < SEQAMOUNT; i++) // For every Cell
		{
			if((current_button_state_register & (1 << i)) == 0 && (previous_button_state_register & (1 << i)) != 0)// && i != line)
			{
				if(global_mode  == SEQMODE)
				{
					pop_from_sequencer(i);
				}
				else if(global_mode == POLYMODE)
				{
					stelement_mute_toggle(i);
				}
				else if(global_mode == MULTMODE)
				{
					mtelement_mute_toggle(i);
				}
				else if(global_mode == TRANSMODE)
				{
					trelement_mute_toggle(i);
				}

			}
		}
		previous_button_state_register = current_button_state_register;

	}
}

void USART3_IRQHandler()
{
	if((USART3->SR & USART_SR_RXNE) != 0)
	{
		byte_receive_from_usart3();
		USART3->SR &= ~USART_SR_RXNE; //Clear RXNE
	}
}

void USART2_IRQHandler()
{
	if((USART2->SR & USART_SR_RXNE) != 0)
	{
		byte_receive_from_usart2();
		USART2->SR &= ~USART_SR_RXNE; //Clear RXNE
	}
}

void USART1_push_to_queue(uint8_t t_byte)
{
	if(tx_usart1_busy == 1)
	{
		// push to the end
		tx_usart1_queue[tx_usart1_end] = t_byte;
		tx_usart1_end++;
	}
	else// if(tx_usart1_busy == 0) // queue is empty
	{
		tx_usart1_busy = 1;
		tx_usart1_index  = 0;
		tx_usart1_end = 1;
		tx_usart1_queue[tx_usart1_index] = t_byte;
		tx_usart1_index++;

		USART1->DR = t_byte;
	}


//	if(tx_usart1_busy == 0) // queue is empty
//	{
//		tx_usart1_busy = 1;
//		tx_usart1_index  = 0;
//		tx_usart1_queue[tx_usart1_index] = t_byte;
//		tx_usart1_end = 1;
//		USART1->DR = t_byte;
//	}
//	else if(tx_usart1_busy == 1)
//	{
//		tx_usart1_queue[tx_usart1_end] = t_byte;
//		tx_usart1_end++;
//	}
}

void USART1_IRQHandler()
{
	if((USART1->SR & USART_SR_TC) != 0)
	{
		if(tx_usart1_busy == 1)
		{
			if(tx_usart1_index < tx_usart1_end)
			{
				USART1->DR = tx_usart1_queue[tx_usart1_index];
				tx_usart1_index++;
			}
			else
			{
				tx_usart1_busy = 0;
				tx_usart1_index  = 0;
				tx_usart1_end = 0;
			}
		}
		USART1->SR &= ~USART_SR_TC;
	}
}


void UART4_push_to_queue(uint8_t t_byte)
{
	if(tx_usart4_busy == 0) // queue is empty
	{
		tx_usart4_busy = 1;
		tx_usart4_index  = 0;
		tx_usart4_end = 1;
		tx_usart4_queue[tx_usart4_index] = t_byte;
		tx_usart4_index++;

		UART4->DR = t_byte;
	}
	else if(tx_usart4_busy == 1)
	{
		tx_usart4_queue[tx_usart4_end] = t_byte;
		tx_usart4_end++;
	}
}

void UART4_IRQHandler()
{
	if((UART4->SR & USART_SR_TC) != 0)
	{
		if(tx_usart4_busy == 1)
		{
			if(tx_usart4_index < tx_usart4_end)
			{
				UART4->DR = tx_usart4_queue[tx_usart4_index++];
			}
			else
			{
				tx_usart4_busy = 0;
				tx_usart4_index  = 0;
				tx_usart4_end = 0;
			}
		}
		UART4->SR &= ~USART_SR_TC;
	}
}


void TIM1_UP_TIM10_IRQHandler()
{
	if((TIM1->SR & TIM_SR_UIF)!= 0)
	{
		//update event every 10 milliseconds
		TIM1->SR &= ~TIM_SR_UIF; 				//Clear Pending Bit

		previous_button_state_register = 0;		//reset buttons
		previous_button_state_register = GPIOC->IDR & 0xFF; //read [0:7] bits of register

		counters_and_flags_control();
		display_show();
	}
}


