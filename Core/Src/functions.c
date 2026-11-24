/*
 * functions.c
 *
 *  Created on: 20 ����. 2021 �.
 *      Author: Vadim
 *
 *
 *
 *
 */
#include "main.h"
#include "functions.h"
#include "at45db_api.h"



////GLOBAL
uint8_t global_mode 		= NDEFMODE;									//0 - sequencer array; 1 - polyphoniser
uint8_t working_mode 		= PLAYMODE;
uint8_t	comm_CC_knob 		= 0;

////LED
struct led learn_led;


uint8_t clkpin_state = 0;

////SEQUENCER
struct midi_container 			sequencer_arr[SEQAMOUNT];				//array of sequencers
extern struct midi_container 	learning_sequencer_buffer;
int 							seq_irq_cnt[SEQAMOUNT];

uint8_t 						active_sequencer_index         	 = 0;
uint8_t 						learning_sequencer_display_shift = 0;	//used in display()


////STACK
struct note_stack 					main_stack;
extern struct note_stack_element 	learning_stelement_buffer;
uint8_t 							learning_stelement_display_dest = 0;//used in display()

struct poly_stack_array				main_plarray;
extern struct poly_stack			learning_plelement_buffer;
uint8_t 							learning_plelement_display_dest = 0;


////MULT
struct note_range_array				main_mult;
extern struct note_range			learning_mtelement_buffer;
uint8_t 							learning_mtelement_display_dest = 0;//used in display()
uint8_t 							border_flag = 0;

////TRANS
struct note_trans_array				main_trans;
struct midi_msg 					temp_trans_note;
struct midi_msg 					temp_trans_cc;
extern struct note_trans			learning_trelement_buffer;
uint8_t 							learning_trelement_display_dest = 0;//used in display()
//uint8_t 							border_flag = 0;

////CONFIG
struct config_arrays main_cfg;

////BYTES & MIDI Messages
uint8_t 		status_byte;
uint8_t 		rx_byte;
uint8_t 		msg_type = 0;
uint8_t         midi_data_flag = 0;

uint8_t			psc_active_cell_cc = 0;

struct midi_msg incoming_note;    							//incoming midi message assembles to this structure
struct midi_msg incoming_note_buffer;	 					//buffer of incoming midi message

struct midi_msg incoming_CC_msg;
struct midi_msg incoming_CC_msg_buffer;

uint8_t 		status_byte2;
uint8_t 		rx_byte2;
uint8_t 		msg_type2 = 0;
uint8_t         midi_data_flag2 = 0;

struct midi_msg incoming_note2;    							//incoming midi message assembles to this structure
struct midi_msg incoming_CC_msg2;




struct midi_msg trigger_list[WAITHEIGHT];					//array of trigger notes.
uint8_t 		trigger_push_index           = 0;
uint8_t 		trigger_pop_index            = 0;

struct midi_msg seq_msg_list[WAITHEIGHT];					//array of sequencer messages need to be sensded.
uint8_t 		seq_msg_push_index           = 0;
uint8_t 		seq_msg_pop_index            = 0;
int 			midi_access_semaph			 = 0;


uint8_t 		rx_byte_waiting_list[MIDIQUEUEN];					// array of incoming bytes from USART3
uint8_t 		rx_byte_waiting_list2[MIDIQUEUEN];					// array of incoming bytes from USART2
uint8_t 		waiting_bytes_push_index     = 0;
uint8_t 		waiting_bytes_pop_index      = 0;
uint8_t 		waiting_bytes_push_index2    = 0;
uint8_t 		waiting_bytes_pop_index2     = 0;

////DISPLAY & COUNTERS
uint8_t sweep_data[] = {
		0b00000001, 0b00000001, 0b00000010, 0b00000100, 	//4
		0b00001000, 0b00010000, 0b00100000, 0b01000000, 	//8
		0b01000000, 0b10000000, 0b11000000, 0b11100000, 	//12
		0b11110000, 0b01111000, 0b01111100, 0b00011100, 	//16
		0b00000110, 0b00000001, 0b00000000, 0b00000000  	//20
};

uint8_t sweep_index   = 0;
uint8_t sweep_flag    = 0;               					//this flag shows led sweep animation is going on
uint8_t sweep_counter = SWEEPTIME;

uint8_t delete_step_counter  = DELETESTEPTIME;
uint8_t delete_process_flag  				= 0;
uint8_t delete_process_duration         	= 0;
uint8_t delete_complete_flag 				= 0;

uint8_t index_show_flag                  	= 0;
uint8_t index_state                      	= 0; 			//state of index led
uint8_t index_toggle_counter 				= TOGGLETIME;
uint8_t index_blink_repeat_counter        	= INDEXBLINKREPEATS;

uint8_t trigger_counter      				= TRIGGERTIME;
uint8_t sequencers_sent_trigger_flag_register 		= 0;	//shows what sequencers has been sent triggers
uint8_t learn_seq_trigger_toggles					= 0;
uint8_t learn_seq_trigger_flag						= 0;

uint8_t trigger_noteOff_counter				= AUTONOTEOFFTIME;

uint8_t alarm_counter        				= ALARMTIME;
uint8_t alarm_state                         = 0; 			//state of alarm led

uint8_t cfg_index_state						= 0;
uint8_t cfg_index_toggle_counter 			= TOGGLETIME;

uint8_t stack_move_element_trigger_flag_register 	= 0;     //for mult and poly
uint8_t show_mutes_flag 							= 0;
uint8_t show_mutes_repeat_counter 					= 0;

uint8_t time_measure_flag 					= 0;     		//enables interval between noteOn and noteOff measurment
uint8_t gate_interval     					= 0;			//how long gate signal take

uint8_t global_mode_select_counter 			= 0;
uint8_t t_global_mode 						= NDEFMODE;

uint8_t pause_dead_time_counter 			= PAUSEDEADTIME;

uint8_t midiclk_counter 					= MIDICLKCNTSTOP;
uint8_t midiclk_state_counter				= 0;

uint8_t thru_trigger_counter				= TRIGGERTIME;
uint8_t thru_trigger_toggles				= 0;

////END OF DEFINES


void structures_init()
{
	midi_msg_init(&incoming_note);
	midi_msg_init(&incoming_note2);

	sequencer_array_init(&sequencer_arr[0]);
	for(int i = 0; i < SEQAMOUNT; i++)
		seq_irq_cnt[i] = 0;

	note_stack_init(&main_stack);

	note_range_array_init(&main_mult);

	note_trans_array_init(&main_trans);

	cfg_init(&main_cfg);
	//poly_stack_array_init(&main_plarray);


	// Load from memory CFG
	uint8_t data_sz = 8*4; // CFGARR_LEN * MODES_AMOUNT
	uint8_t cfg_data[data_sz];
	at45dblib_wait_ready_bit();
	at45dblib_cont_read(SAVECFGPG, cfg_data, data_sz);

	for(int i = 0; i < 8; i++)
	{
		main_cfg.mult[i] = cfg_data[i];
	}
	for(int i = 0; i < 8; i++)
	{
		main_cfg.stack[i] = cfg_data[i + 8];
	}
	for(int i = 0; i < 8; i++)
	{
		main_cfg.trans[i] = cfg_data[i + 16];
	}
	for(int i = 0; i < 8; i++)
	{
		main_cfg.seq[i] = cfg_data[i + 24];
	}

    // Load from memory Srtuctures
	if(global_mode == TRANSMODE)
	{
		uint8_t t_i = main_cfg.active_index;
		uint8_t t_d = main_cfg.trans[t_i];
		midi_dac_switch_cfg(t_i, t_d);

		//note_trans_array_print(&main_trans);
		uint8_t data_sz = STACKHEIGHT*5+1;
		uint8_t trans_data[data_sz];
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVETRANSPG, trans_data, data_sz);

		for(int i = 0; i < TRANSCCAMOUNT; i++)
		{
			main_trans.trelement[i].en 						= trans_data[i];
			main_trans.trelement[i].parameters[TRANSCCPos] 	= trans_data[i+TRANSCCAMOUNT];
			main_trans.trelement[i].parameters[TRANSSRCPos] = trans_data[i+2*TRANSCCAMOUNT];
			main_trans.trelement[i].parameters[TRANSDSTPos] = trans_data[i+3*TRANSCCAMOUNT];
			main_trans.trelement[i].open 					= trans_data[i+4*TRANSCCAMOUNT];
		}
		main_trans.active_trindex =  trans_data[data_sz - 1];

	}
	if(global_mode == MULTMODE)
	{
		uint8_t t_i = main_cfg.active_index;
		uint8_t t_d = main_cfg.mult[t_i];
		midi_dac_switch_cfg(t_i, t_d);

		//note_range_array_print(&main_mult);
		uint8_t data_sz = STACKHEIGHT*6+1;
		uint8_t mult_data[data_sz];
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVEMULTPG, mult_data, data_sz);

		for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		{
			main_mult.mtelement[i].en 							= mult_data[i];
			main_mult.mtelement[i].destination 					= mult_data[i+MULTTIMBREAMOUNT];
			main_mult.mtelement[i].parameters[MULTSTARTPos] 	= mult_data[i+2*MULTTIMBREAMOUNT];
			main_mult.mtelement[i].parameters[MULTENDPos]   	= mult_data[i+3*MULTTIMBREAMOUNT];
			main_mult.mtelement[i].parameters[MULTRANGESHIFTPos]= mult_data[i+4*MULTTIMBREAMOUNT];
			main_mult.mtelement[i].open 						= mult_data[i+5*MULTTIMBREAMOUNT];
		}
		main_mult.active_mtindex = mult_data[data_sz - 1];
	}
	if(global_mode == POLYMODE)
	{
		uint8_t t_i = main_cfg.active_index;
		uint8_t t_d = main_cfg.stack[t_i];
		midi_dac_switch_cfg(t_i, t_d);

		uint8_t data_sz = STACKHEIGHT*3+1;
		uint8_t poly_data[data_sz];
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVEPOLYPG, poly_data, data_sz);

		for(int i = 0; i < STACKHEIGHT; i++)
		{
			main_stack.stelement[i].destination = poly_data[i];
			main_stack.stelement[i].en = poly_data[i+STACKHEIGHT];
			main_stack.stelement[i].open = poly_data[i+2*STACKHEIGHT];
		}
		main_stack.active_stindex = poly_data[data_sz - 1];
		//poly_stack_array_print(&main_plarray);
		//note_stack_print(&main_stack);
	}
	if(global_mode == SEQMODE)
	{
		uint8_t t_i = main_cfg.active_index;
		uint8_t t_d = main_cfg.seq[t_i];
		midi_dac_switch_cfg(t_i, t_d);

		uint16_t data_sz = 256; // SEQAMOUNT/2*STEPSINSEQ*4bytes
		uint8_t seq_data[data_sz];
		// Load first 4 sequencers
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVESEQPG0, seq_data, data_sz);
		for(int i = 0; i < SEQAMOUNT>>1; i++) 	// i - number of Sequencer
		{
			for(int j = 0; j < 16; j++) 		// j - number of MIDI Note
			{
				sequencer_arr[i].note[j].status = seq_data[i*64+j*4];
				sequencer_arr[i].note[j].data1 	= seq_data[i*64+j*4+1];
				sequencer_arr[i].note[j].data2 	= seq_data[i*64+j*4+2];
				sequencer_arr[i].note[j].trig 	= seq_data[i*64+j*4+3];
			}

		}
		// Load second 4 sequencers
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVESEQPG1, seq_data, data_sz);
		for(int i = 0; i < SEQAMOUNT>>1; i++) 	// i - number of Sequencer
		{
			for(int j = 0; j < 16; j++) 		// j - number of MIDI Note
			{
				sequencer_arr[i+4].note[j].status   = seq_data[i*64+j*4];
				sequencer_arr[i+4].note[j].data1 	= seq_data[i*64+j*4+1];
				sequencer_arr[i+4].note[j].data2 	= seq_data[i*64+j*4+2];
				sequencer_arr[i+4].note[j].trig 	= seq_data[i*64+j*4+3];
			}
		}
		// Load remain Data
		data_sz = 8 + 1;
		at45dblib_wait_ready_bit();
		at45dblib_cont_read(SAVESEQPG2, seq_data, data_sz);
		for(int i = 0 ; i < SEQAMOUNT; i++)
		{
			sequencer_arr[i].last_index = seq_data[i];
		}
		active_sequencer_index = seq_data[data_sz - 1];

		//poly_stack_array_print(&main_plarray);
		//note_stack_print(&main_stack);
	}
	/////////////////////

	//note_stack_print(&main_stack); //debug

}

void cfg_store_to_at45db()
{
	// Store to memory CFG
	uint8_t data_sz = 8*4;    // CFGARR_LEN * MODES_AMOUNT
	uint8_t cfg_data[data_sz];

	for(int i = 0; i < 8; i++)
	{
		cfg_data[i]      = main_cfg.mult[i];
	}
	for(int i = 0; i < 8; i++)
	{
		cfg_data[i + 8]  = main_cfg.stack[i];
	}
	for(int i = 0; i < 8; i++)
	{
		cfg_data[i + 16] = main_cfg.trans[i];
	}
	for(int i = 0; i < 8; i++)
	{
		cfg_data[i + 24] = main_cfg.seq[i];
	}
	at45dblib_wait_ready_bit();
	at45dblib_page_prog_1(SAVECFGPG, cfg_data, data_sz);
}



//////////////DISPLAY

void display_show() 										//function calculating one frame every 10ms of TIM1 update irq
{
	uint8_t display_data = 0; 								//0b number with led display state
	if(global_mode == NDEFMODE && t_global_mode != NDEFMODE)
	{
		display_data = 1 << t_global_mode;
	}
	else if     (sweep_flag == 1)
	{
		display_data = sweep_data[sweep_index];
	}
	//Всю дальнейшую колбасу переписать с учетом того, что будут добавляться режимы работы, а play learn cfg будут постоянны
	else if(working_mode == PLAYMODE) 					//no sequencer or stack_element in learn mode
	{
		LEARNLEDPORT->ODR &= ~(1 << LEARNLEDPIN);
		if(thru_trigger_toggles != 0)
		{
			LEARNLEDPORT->ODR |= (1 << LEARNLEDPIN);
		}


//		uint8_t t_clk_pol;

		if(global_mode == SEQMODE)
		{
			display_data = sequencer_work_display_data();
//			t_clk_pol = main_cfg.seq[MIDICLKTHRUPos];
		}
		else if(global_mode == POLYMODE)
		{
			display_data = poly_work_display_data();
//			t_clk_pol = main_cfg.stack[MIDICLKTHRUPos];
		}
		else if(global_mode == MULTMODE)
		{
			display_data = mult_work_display_data();
//			t_clk_pol = main_cfg.mult[MIDICLKTHRUPos];
		}
		else if(global_mode == TRANSMODE)
		{
			display_data = trans_work_display_data();
//			t_clk_pol = main_cfg.trans[MIDICLKTHRUPos];
		}


/*		if(t_clk_pol == MIDInCLKPOL)
		{
			CLKLEDPORT->ODR |= (1 << CLKLEDPIN);				//MIDI Sync Pin Set - Default State
		}
		else if (t_clk_pol == MIDICLKPOL)
		{
			CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);					//MIDI Sync Pin Reset - Default State
		}
*/
		//CLKLEDPORT->ODR |= 1 << CLKLEDPIN; 				//MIDI Sync LED Set - Default state
		//CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);
		//CLKLEDPORT->ODR |= clkpin_state << CLKLEDPIN;
	}
	else if(working_mode == LEARNMODE)                    			// sequencer or stelement in learn mode
	{
		LEARNLEDPORT->ODR |= 1 << LEARNLEDPIN;

//		uint8_t t_clk_pol;

		if(global_mode == SEQMODE)
		{

			if(delete_process_flag == 1)
			{
				display_data = 0b11111111 << ((STEPSINSEQ - (delete_process_duration)) >> 1);
			}
			else
			{
				display_data  = 0b11111111 << ((learning_sequencer_display_shift + 1) >> 1);
				if(learning_sequencer_display_shift % 2 == 0)
					display_data |= learn_seq_trigger_flag << (((learning_sequencer_display_shift + 1) >> 1)-1);
			}
//			t_clk_pol = main_cfg.seq[MIDICLKTHRUPos];
		}
		else if(global_mode == POLYMODE)
		{

			if(delete_process_flag == 1)
			{
				display_data = 0b11111111 << ((STEPSINSEQ - (delete_process_duration)) >> 1);
			}
			else
			{
				display_data = ~learning_stelement_display_dest;
			}
//			t_clk_pol = main_cfg.stack[MIDICLKTHRUPos];
		}
		else if(global_mode == MULTMODE)
		{

			if(delete_process_flag == 1)
			{
				display_data = 0b11111111 << ((STEPSINSEQ - (delete_process_duration)) >> 1);
			}
			else
			{
				display_data = ~learning_mtelement_display_dest;
			}
//			t_clk_pol = main_cfg.mult[MIDICLKTHRUPos];
		}
		else if(global_mode == TRANSMODE)
		{
			struct note_trans active_translator;
			uint8_t active_param_index;
			active_param_index = main_trans.trelement[main_trans.active_trindex].active_trparamindex;
			active_translator = main_trans.trelement[main_trans.active_trindex];

			if(delete_process_flag == 1)
			{
				display_data = 0b11111111 << ((STEPSINSEQ - (delete_process_duration)) >> 1);
			}
			else
			{
				if(active_param_index == TRANSCCPos)
				{
					display_data = (cfg_index_state << active_param_index) | (~(active_translator.parameters[active_param_index] << 1)&0b11111110);
				}
				else
				{
					uint8_t high;
					high = ((active_param_index >> 2) == 0) ? 1 : 0;

//					display_data = ((1 << active_param_index)^(0xF * cfg_index_state)) | (~((active_translator.parameters[active_param_index] + 1)<< (high << 2)))&0xF0;
					display_data = (1 << active_param_index)^(0xF * cfg_index_state);

//					display_data = 0xF & (~(cfg_index_state * (1 << active_param_index)));
					display_data|= (~((active_translator.parameters[active_param_index] + active_translator.en)<< (high << 2)))&0xF0;
				}
			}
//			t_clk_pol = main_cfg.trans[MIDICLKTHRUPos];
		}
/*
		if(t_clk_pol == MIDInCLKPOL)
		{
			CLKLEDPORT->ODR |= (1 << CLKLEDPIN);				//MIDI Sync Pin Set - Default State
		}
		else if (t_clk_pol == MIDICLKPOL)
		{
			CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);					//MIDI Sync Pin Reset - Default State
		}
*/
		//CLKLEDPORT->ODR |= 1 << CLKLEDPIN; 				//MIDI Sync LED Set - Default state
		//CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);
		//CLKLEDPORT->ODR |= clkpin_state << CLKLEDPIN;

	}
	else if(working_mode == CFGMODE)                    			// sequencer or stelement in learn mode
	{
		LEARNLEDPORT->ODR &= ~(1 << LEARNLEDPIN);
		LEARNLEDPORT->ODR |=   index_state << LEARNLEDPIN ;

//		uint8_t t_clk_pol;
		//CLKLEDPORT->ODR |= 1 << CLKLEDPIN;
		//CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);
		//CLKLEDPORT->ODR |= clkpin_state << CLKLEDPIN;
		if(global_mode == SEQMODE)
		{
			uint8_t high;
			uint8_t t_index;
			t_index = main_cfg.active_index;
			high = ((t_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << t_index;
			if(t_index == MIDICLKPSCPos)
			{
				display_data |= (main_cfg.seq[t_index] + 1) << (high << 2) ;
			}
			else
			{
				display_data |= (main_cfg.seq[t_index]) << (high << 2) ;
			}
			/*
			uint8_t high;
			high = ((main_cfg.active_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << main_cfg.active_index
						 | main_cfg.seq[main_cfg.active_index] << (high << 2) ;
			*/
//			t_clk_pol = main_cfg.seq[MIDICLKTHRUPos];
		}
		else if(global_mode == POLYMODE)
		{
			uint8_t high;
			uint8_t t_index;
			t_index = main_cfg.active_index;
			high = ((t_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << t_index;
			if(t_index == POLYINCHPos)
			{
				display_data |= (main_cfg.stack[t_index] + 1) << (high << 2) ;
			}
			else if(t_index == MIDICLKPSCPos)
			{
				display_data |= (main_cfg.stack[t_index] + 1) << (high << 2) ;
			}
			else
			{
				display_data |= (main_cfg.stack[t_index]) << (high << 2) ;
			}

//			t_clk_pol = main_cfg.stack[MIDICLKTHRUPos];
		}
		else if(global_mode == MULTMODE)
		{
			uint8_t high;
			uint8_t t_index;
			t_index = main_cfg.active_index;
			high = ((t_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << t_index;
			if(t_index == MULTINCHPos)
			{
				display_data |= (main_cfg.mult[t_index] + 1) << (high << 2) ;
			}
			else if(t_index == MIDICLKPSCPos)
			{
				display_data |= (main_cfg.mult[t_index] + 1) << (high << 2) ;
			}
			else
			{
				display_data |= (main_cfg.mult[t_index]) << (high << 2) ;
			}


			/*
			uint8_t high;
			high = ((main_cfg.active_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << main_cfg.active_index
						 | main_cfg.mult[main_cfg.active_index] << (high << 2) ;
			*/
//			t_clk_pol = main_cfg.mult[MIDICLKTHRUPos];
		}
		else if(global_mode == TRANSMODE)
		{
			uint8_t high;
			uint8_t t_index;
			t_index = main_cfg.active_index;
			high = ((t_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << t_index;
			if(t_index == MIDICLKPSCPos)
			{
				display_data |= (main_cfg.trans[t_index] + 1) << (high << 2) ;
			}
			else
			{
				display_data |= (main_cfg.trans[t_index]) << (high << 2) ;
			}


			/*
			uint8_t high;
			high = ((main_cfg.active_index >> 2) == 0) ? 1 : 0;

			display_data = cfg_index_state << main_cfg.active_index
						 | main_cfg.trans[main_cfg.active_index] << (high << 2) ;
			*/

//			t_clk_pol = main_cfg.trans[MIDICLKTHRUPos];
		}


/*
		if(t_clk_pol == MIDInCLKPOL)
		{
			CLKLEDPORT->ODR |= (1 << CLKLEDPIN);				//MIDI Sync Pin Set - Default State
		}
		else if (t_clk_pol == MIDICLKPOL)
		{
			CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);					//MIDI Sync Pin Reset - Default State
		}
*/
		//CLKLEDPORT->ODR |= 1 << CLKLEDPIN; 				//MIDI Sync LED Set - Default state
		//CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);
		//CLKLEDPORT->ODR |= clkpin_state << CLKLEDPIN;
	}
	led_display_config_and_show(display_data);
}
uint8_t sequencer_work_display_data()
{
	uint8_t data = 0;
	if(index_show_flag == 1)
	{
		data =	(index_state << active_sequencer_index);
		return data;
	}
	else
	{
		for(int i = 0; i < SEQAMOUNT; i++)
		{
			if(sequencer_arr[i].active_notes != 0)
			{
				if(sequencer_arr[i].active_notes < 18)
					data |= 1 <<  i;
				else if(sequencer_arr[i].active_notes >= 18)
					data |= alarm_state <<  i; // alarm_state - ������ ���������� ��������
			}
		}
		if(sequencers_sent_trigger_flag_register != 0)
			data ^= sequencers_sent_trigger_flag_register;
		return data;
	}


}
uint8_t poly_work_display_data()
{
	uint8_t data = 0;
	if(show_mutes_flag == 0)
	{
		for(int i = 0; i < STACKHEIGHT; i++)
		{
			if(main_stack.stelement[i].active != 0)
				data |= 1 << i;
		}
		if(stack_move_element_trigger_flag_register != 0)
			data ^= stack_move_element_trigger_flag_register;
	}
	else
	{
		for(int i = 0; i < STACKHEIGHT; i++)
		{
			if(main_stack.stelement[i].open != 0 && main_stack.stelement[i].en != 0)
				data |= 1 << i;
		}
	}
	if(index_show_flag == 1)
	{
		//active_notes_data &= ~(1 << main_stack.active_stindex); 		//clear blinking sequencer bit
		data = (index_state << main_stack.active_stindex);
		return data;
		//				 	  | active_notes_data);
	}
	else
		return data;
}
uint8_t mult_work_display_data()
{
	uint8_t data = 0;
	if(show_mutes_flag == 0)
	{
		for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		{
			if(main_mult.mtelement[i].active != 0)
				data |= 1 << i;
		}
		if(stack_move_element_trigger_flag_register != 0)
			data ^= stack_move_element_trigger_flag_register;
	}
	else
	{
		for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		{
			if(main_mult.mtelement[i].open != 0 && main_mult.mtelement[i].en != 0)
				data |= 1 << i;
		}
	}
	if(index_show_flag == 1)
	{
		//active_notes_data &= ~(1 << main_stack.active_stindex); 		//clear blinking sequencer bit
		data = (index_state << main_mult.active_mtindex);
		return data;
		//				 	  | active_notes_data);
	}
	else
		return data;
}
uint8_t trans_work_display_data()
{
	uint8_t data = 0;
	if(show_mutes_flag == 0)
	{
		for(int i = 0; i < TRANSCCAMOUNT; i++)
		{
			if(main_trans.trelement[i].active != 0)
				data |= 1 << i;
		}
		if(stack_move_element_trigger_flag_register != 0)
			data ^= stack_move_element_trigger_flag_register;
	}
	else
	{
		for(int i = 0; i < TRANSCCAMOUNT; i++)
		{
			if(main_trans.trelement[i].open != 0 && main_trans.trelement[i].en != 0)
				data |= 1 << i;
		}
	}
	if(index_show_flag == 1)
	{
		//active_notes_data &= ~(1 << main_stack.active_stindex); 		//clear blinking sequencer bit
		data = (index_state << main_trans.active_trindex);
		return data;
		//				 	  | active_notes_data);
	}
	else
		return data;
}
uint8_t learning_sequencer_last_index(struct midi_container learning_sequencer, struct midi_container learning_sequencer_buffer)
{

	if(learning_sequencer.last_index == 0)
		return learning_sequencer_buffer.last_index;
	else
		return learning_sequencer.last_index;
}
uint8_t learning_stelement_destination(struct note_stack_element learning_stelement, struct note_stack_element learning_stelement_buffer)
{
	if(learning_stelement.en == 0)
		return learning_stelement_buffer.destination + learning_stelement_buffer.en;
	else
		return learning_stelement.destination + learning_stelement.en;

}
uint8_t learning_mtelement_destination(struct note_range learning_mtelement, struct note_range learning_mtelement_buffer)
{
	if(learning_mtelement.en == 0)
	{
		if((GPIOC->IDR & (1 << CFGBTNPIN)) != 0)// not pressed
		{
			return learning_mtelement_buffer.destination + learning_mtelement_buffer.en + ((learning_mtelement.parameters[MULTRANGESHIFTPos])<<4);
		}
		else
		{
			uint8_t data = 0;
			data = ((learning_mtelement_buffer.parameters[MULTENDPos] - learning_mtelement_buffer.parameters[MULTSTARTPos]) >> 4) + 1;
			data = 0xFF >> (8 - data);
			data = data << (learning_mtelement_buffer.parameters[MULTSTARTPos] >> 4);
			return data;
		}

	}
	else
	{
		if((GPIOC->IDR & (1 << CFGBTNPIN)) != 0)// not pressed
		{
			return learning_mtelement.destination + learning_mtelement.en + ((learning_mtelement.parameters[MULTRANGESHIFTPos])<<4);
		}
		else
		{
			uint8_t data = 0;
			data = ((learning_mtelement.parameters[MULTENDPos] - learning_mtelement.parameters[MULTSTARTPos]) >> 4) + 1;
			data = 0xFF >> (8 - data);
			data = data << (learning_mtelement.parameters[MULTSTARTPos] >> 4);
			return data;
		}
//		return (learning_mtelement.destination + learning_stelement_buffer.en + ((learning_mtelement.parameters[MULTRANGESHIFTPos])<<4));
	}

}


void counters_and_flags_control()
{
	learning_sequencer_display_shift = learning_sequencer_last_index(sequencer_arr[active_sequencer_index],
																	 learning_sequencer_buffer);
	learning_stelement_display_dest = learning_stelement_destination(main_stack.stelement[main_stack.active_stindex],
																	   learning_stelement_buffer);
	learning_mtelement_display_dest = learning_mtelement_destination(main_mult.mtelement[main_mult.active_mtindex],
																		   learning_mtelement_buffer);

	index_blink_control();
	cfg_index_blink_control();
	global_mode_select_counter_control();
	sweep_animation_phase_control();

	midi_clk_state_control();

	thru_trigger_counter_control();
	seq_trigger_noteOff_counter_control(); //��� seq � ��� mult

	/////COUNTERS depended on MODE
	if(global_mode == SEQMODE)
	{
		seq_alarm_counter_control();
		seq_delete_counter_control();
		seq_trigger_counter_control();
		pause_dead_time_counter_control();
	}
	else if(global_mode == POLYMODE)
	{
		poly_delete_step_counter_control();
		poly_trigger_counter_control();
	}
	else if(global_mode == MULTMODE)
	{
		mult_delete_step_counter_control();
		mult_trigger_counter_control();
	}
	else if(global_mode == TRANSMODE)
	{
		trans_delete_step_counter_control();
		trans_trigger_counter_control();
	}
    //increasing only then  time_measure_flag is set
	gate_interval = (gate_interval < TOGGLETIME)?(gate_interval + time_measure_flag) : TOGGLETIME;
}
////GLOBAL Counters
////INDEX
void index_blink_control()
{
	if(--index_toggle_counter == 0)
	{
		index_state = (index_state == 0) ? 1 : 0; //toggle index_state

		if(index_show_flag == 1)
		{
			if(--index_blink_repeat_counter == 0)
				index_show_flag = 0;
		}
		//��� ������ ���� ��������� �������
		if(show_mutes_flag == 1)
		{
			if(--show_mutes_repeat_counter == 0)
				show_mutes_flag = 0;
		}

		index_toggle_counter = TOGGLETIME;
	}
}
void index_show_set()
{
	index_state 			 	= 1;
	index_toggle_counter		= TOGGLETIME;
	index_show_flag 		   	= 1;
	index_blink_repeat_counter 	= INDEXBLINKREPEATS;
}
////CFG
void cfg_index_blink_control()
{
	if(--cfg_index_toggle_counter == 0)
	{
		cfg_index_state = (cfg_index_state == 0) ? 1 : 0; //toggle cfg_index_state
		cfg_index_toggle_counter = TOGGLETIME;
	}
}
void global_mode_select_counter_control()
{
	if(global_mode == NDEFMODE && global_mode_select_counter == 0)
	{
		global_mode = t_global_mode;
	}
	else
	{
		global_mode_select_counter--;
	}
}
void midi_clk_state_control()
{

	if(midiclk_state_counter != 0)
	{
		midiclk_state_counter--;
	}
	else
	{
		uint8_t t_clk_pol;
		if(global_mode == SEQMODE)
		{
			t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
		}
		else if(global_mode == POLYMODE)
		{
			t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
		}
		else if(global_mode == MULTMODE)
		{
			t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
		}
		else if(global_mode == TRANSMODE)
		{
			t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
		}

		/////// Control of CLK PIN state
		if(t_clk_pol == MIDInCLKPOL)
		{
			CLKLEDPORT->ODR |= (1 << CLKLEDPIN);				//MIDI Sync Pin Set - Default State
		}
		else if (t_clk_pol == MIDICLKPOL)
		{
			CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);					//MIDI Sync Pin Reset - Default State
		}
		////////
	}
}
////SWEEP
void sweep_animation_phase_control()
{
	if(--sweep_counter == 0)
	{
		if(sweep_flag == 1)
			if(++sweep_index == SWEEPELEMENTS)
			{
				sweep_show_reset();
				index_show_set(); //after end of learn
			}
		sweep_counter = SWEEPTIME;
	}
}
void sweep_show_reset()
{
	sweep_index = 0;
	sweep_flag =  0;
}

/////MUTES
void show_mutes_set()
{
	show_mutes_flag = 1;
	show_mutes_repeat_counter  = MUTESREPEATS;
}
/////DELETE
void seq_delete_counter_control()
{
	if(--delete_step_counter == 0)
	{
		if(working_mode == LEARNMODE) //sequencer in learn mode DELETE
		{
			if((GPIOC->IDR & 1 << LEARNBTNPIN) == 0) // learn pressed in learn mode
			{
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0)
					delete_process_flag = 1;
				if(delete_process_duration == STEPSINSEQ)
					delete_complete_flag = 1;
				/*
				if(delete_complete_flag == 0) // wait for 4 cycles
					delete_process_duration++;
				if(delete_process_flag == 0 && delete_process_duration == 4)
					delete_process_flag = 1;
				else if((delete_process_duration - 4) == (learning_sequencer_display_shift + 1))
					delete_complete_flag = 1;
				*/
			}
			else
				delete_procedure_reset();
		}
		delete_step_counter = DELETESTEPTIME;
	}
}
void poly_delete_step_counter_control()
{
	if(--delete_step_counter == 0)
	{
		if(working_mode == LEARNMODE) //sequencer or stelement in learn mode DELETE
		{
			if((GPIOC->IDR & 1 << LEARNBTNPIN) == 0)
			{
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0)
					delete_process_flag = 1;
				else if(delete_process_duration == STEPSINSEQ)
					delete_complete_flag = 1;
			}
			else
				delete_procedure_reset();
		}
		delete_step_counter = DELETESTEPTIME;
	}

}
void mult_delete_step_counter_control()
{
	if(--delete_step_counter == 0)
	{
		if(working_mode == LEARNMODE) //sequencer or stelement in learn mode DELETE
		{
			if((GPIOC->IDR & (1 << LEARNBTNPIN)) == 0) // pressed
			{
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0)
					delete_process_flag = 1;
				else if(delete_process_duration == STEPSINSEQ)
					delete_complete_flag = 1;
				/*
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0 && delete_process_duration == 4)
					delete_process_flag = 1;
				else if((delete_process_duration - 4) == (learning_mtelement_buffer.destination + 1))
					delete_complete_flag = 1;*/
			}
			//else
			//	delete_procedure_reset();
		}
		delete_step_counter = DELETESTEPTIME;
	}

}
void trans_delete_step_counter_control()
{
	if(--delete_step_counter == 0)
	{
		if(working_mode == LEARNMODE) //sequencer or stelement in learn mode DELETE
		{
			if((GPIOC->IDR & 1 << LEARNBTNPIN) == 0)
			{
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0)
					delete_process_flag = 1;
				else if(delete_process_duration == STEPSINSEQ)
					delete_complete_flag = 1;

				/*
				if(delete_complete_flag == 0)
					delete_process_duration++;
				if(delete_process_flag == 0 && delete_process_duration == 4)
					delete_process_flag = 1;
				else if((delete_process_duration - 4) == (learning_trelement_buffer.destination + 1))
					delete_complete_flag = 1;*/
			}
			else
				delete_procedure_reset();
		}
		delete_step_counter = DELETESTEPTIME;
	}

}
void delete_procedure_reset()
{
	delete_process_flag     = 0;
	delete_process_duration = 0;
	delete_complete_flag    = 0;
}

/////TRIGGER NOTEOFF
void empty_trig_queue()
{
	if(trigger_noteOff_counter == 0)
	{
		// Empty Trigger Queue
		while(trigger_pop_index < trigger_push_index)
		{
			send_noteOff_for_noteOn(trigger_list[trigger_pop_index]);
			midi_msg_init(&trigger_list[trigger_pop_index]);
			trigger_pop_index++;
		}
		trigger_push_index = 0;
		trigger_pop_index  = 0;
	}
}

void seq_trigger_noteOff_counter_control()
{
	if(trigger_noteOff_counter != 0)
	{
		trigger_noteOff_counter--;
		/*
		if(trigger_noteOff_counter == 0)
		{
			// Empty Trigger Queue
			while(trigger_pop_index < trigger_push_index)
			{
				send_noteOff_for_noteOn(trigger_list[trigger_pop_index]);
				midi_msg_init(&trigger_list[trigger_pop_index]);
				trigger_pop_index++;
			}
			trigger_push_index = 0;
			trigger_pop_index  = 0;
		}
		*/
	}
}
void trigger_noteOff_counter_set()
{
	trigger_noteOff_counter = AUTONOTEOFFTIME;
}


void seq_alarm_counter_control()
{
	if(--alarm_counter == 0)
	{
		alarm_state = (alarm_state == 0) ? 1 : 0; //toggle alarm_state
		alarm_counter = ALARMTIME;
	}

}
////TRIGGER
void seq_trigger_counter_control()
{
	if(--trigger_counter == 0)
	{
		for(int j = 0; j < SEQAMOUNT; j++)
			if(sequencer_arr[j].trigger_toggles != 0)
			{
				sequencer_arr[j].trigger_toggles--; //decrementing inside to avoid constant blinking
				sequencers_sent_trigger_flag_register ^= 1 << j; //toggle flag state
			}
		if(learn_seq_trigger_toggles != 0)
		{
			learn_seq_trigger_toggles--;
			learn_seq_trigger_flag = (learn_seq_trigger_flag == 1)? 0 : 1;
		}
		trigger_counter = TRIGGERTIME;
	}
}
void poly_trigger_counter_control()
{
	if(--trigger_counter == 0)
	{
		for(int j = 0; j < STACKHEIGHT; j++)
			if(main_stack.stelement[j].moving_toggles != 0)
			{
				main_stack.stelement[j].moving_toggles--; //decrementing inside to avoid constant blinking
				stack_move_element_trigger_flag_register ^= 1 << j; //toggle flag state
			}
		trigger_counter = TRIGGERTIME;
	}
}
void mult_trigger_counter_control()
{
	if(--trigger_counter == 0)
	{
		for(int j = 0; j < MULTTIMBREAMOUNT; j++)
			if(main_mult.mtelement[j].moving_toggles != 0)
			{
				main_mult.mtelement[j].moving_toggles--; //decrementing inside to avoid constant blinking
				stack_move_element_trigger_flag_register ^= 1 << j; //toggle flag state
			}
		trigger_counter = TRIGGERTIME;
	}
}
void trans_trigger_counter_control()
{
	if(--trigger_counter == 0)
	{
		for(int j = 0; j < TRANSCCAMOUNT; j++)
			if(main_trans.trelement[j].moving_toggles != 0)
			{
				main_trans.trelement[j].moving_toggles--; //decrementing inside to avoid constant blinking
				stack_move_element_trigger_flag_register ^= 1 << j; //toggle flag state
			}
		trigger_counter = TRIGGERTIME;
	}
}

void thru_trigger_counter_control()
{
	if(--thru_trigger_counter == 0)
	{
		if(thru_trigger_toggles != 0)
			thru_trigger_toggles--;
		thru_trigger_counter = TRIGGERTIME;
	}
}
void thru_trigger_counter_set()
{
	thru_trigger_toggles++;
	thru_trigger_counter = TRIGGERTIME;
}
////PAUSE APPEND
void pause_dead_time_counter_control()
{
	if(pause_dead_time_counter != 0)
	{
		pause_dead_time_counter--;
	}
}


/////////////
////SEQUENCER
void sequencer_learn_mode_toggle()
{
	if(sequencer_arr[active_sequencer_index].learn == 0)
	{
		sequncer_start_of_learn(&sequencer_arr[active_sequencer_index]);
	}
	else
	{
		sequencer_end_of_learn(&sequencer_arr[active_sequencer_index]);

		//Saving to flash if CFG is pressed
		if((GPIOC->IDR & 1 << CFGBTNPIN) == 0)
		{
			uint16_t data_sz = 256; // SEQAMOUNT/2*STEPSINSEQ*4bytes
			uint8_t seq_data[data_sz];
			// Store first 4 sequencers
			for(int i = 0; i < SEQAMOUNT>>1; i++) 	// i - number of Sequencer
			{
				for(int j = 0; j < 16; j++) 		// j - number of MIDI Note
				{
					seq_data[i*64+j*4]    = sequencer_arr[i].note[j].status;
					seq_data[i*64+j*4+1]  = sequencer_arr[i].note[j].data1;
					seq_data[i*64+j*4+2]  = sequencer_arr[i].note[j].data2;
					seq_data[i*64+j*4+3]  = sequencer_arr[i].note[j].trig;
				}

			}
			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVESEQPG0, seq_data, data_sz);
			// Store second 4 sequencers
			for(int i = 0; i < SEQAMOUNT>>1; i++) 	// i - number of Sequencer
			{
				for(int j = 0; j < 16; j++) 		// j - number of MIDI Note
				{
					seq_data[i*64+j*4]    = sequencer_arr[i+4].note[j].status;
					seq_data[i*64+j*4+1]  = sequencer_arr[i+4].note[j].data1;
					seq_data[i*64+j*4+2]  = sequencer_arr[i+4].note[j].data2;
					seq_data[i*64+j*4+3]  = sequencer_arr[i+4].note[j].trig;
				}

			}
			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVESEQPG1, seq_data, data_sz);
			// Store remain Data
			data_sz = 8 + 1;
			for(int i = 0 ; i < SEQAMOUNT; i++)
			{
				seq_data[i] = sequencer_arr[i].last_index;
			}
			seq_data[data_sz - 1] = active_sequencer_index;
			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVESEQPG2, seq_data, data_sz);

		}
		//midi_container_print(&sequencer_arr[active_sequencer_index], active_sequencer_index); //debug
	}
}
void pop_from_sequencer(int index)
{
	seq_irq_cnt[index]++;
	//real_pop_from_sequencer(&sequencer_arr[index], index);
}
void next_sequencer()
{
	real_next_sequencer(&sequencer_arr[active_sequencer_index]);
}
void prev_sequencer()
{
	real_prev_sequencer(&sequencer_arr[active_sequencer_index]);
}
/////////
////STACK
void note_stack_learn_mode_toggle()
{
	if(main_stack.stelement[main_stack.active_stindex].learn == 0)
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
			stack_start_of_learn(&main_stack.stelement[main_stack.active_stindex]);
		}

		//stack_start_of_learn(&main_stack.stelement[main_stack.active_stindex]);
	}
	else
	{
		stack_end_of_learn(&main_stack.stelement[main_stack.active_stindex]);

		//Saving to flash if CFG is pressed
		if((GPIOC->IDR & 1 << CFGBTNPIN) == 0)
		{
			uint8_t data_sz = STACKHEIGHT*3 +1;
			uint8_t poly_data[data_sz];
			for(int i = 0; i < STACKHEIGHT; i++)
			{
				poly_data[i] = main_stack.stelement[i].destination;
				poly_data[i+STACKHEIGHT] = main_stack.stelement[i].en ;
				poly_data[i+2*STACKHEIGHT] = main_stack.stelement[i].open;
			}
			poly_data[data_sz - 1] = main_stack.active_stindex;


			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVEPOLYPG, poly_data, data_sz);
		}


		//note_stack_print(&main_stack);//debug

	}
}
void stelement_mute_toggle(int index)
{
	real_stelement_mute_toggle(&main_stack, &main_stack.stelement[index], index);
}
void prev_stelement()
{
	real_prev_stelement(&main_stack.stelement[main_stack.active_stindex], &main_stack.active_stindex);
}
void next_stelement()
{
	real_next_stelement(&main_stack.stelement[main_stack.active_stindex], &main_stack.active_stindex);
}
/////////
////MULT
void prev_mtelement()
{
	real_prev_mtelement(&main_mult);
}
void next_mtelement()
{
	real_next_mtelement(&main_mult);
}

void start_range_note_trigger()
{
	real_start_range_note_trigger(&main_mult.mtelement[main_mult.active_mtindex]);
}
void end_range_note_trigger()
{
	real_end_range_note_trigger(&main_mult.mtelement[main_mult.active_mtindex]);
}
void prev_mtparam()
{
	real_prev_mtparam(&main_mult.mtelement[main_mult.active_mtindex]);
}
void next_mtparam()
{
	real_next_mtparam(&main_mult.mtelement[main_mult.active_mtindex]);
}
void note_range_learn_mode_toggle()
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
			mult_start_of_learn(&main_mult.mtelement[main_mult.active_mtindex]);
		}
		//mult_start_of_learn(&main_mult.mtelement[main_mult.active_mtindex]);
		//working_mode = LEARNMODE;
	}
	else
	{
		mult_end_of_learn(&main_mult.mtelement[main_mult.active_mtindex]);

		if((GPIOC->IDR & 1 << CFGBTNPIN) == 0)
		{
			uint8_t data_sz = MULTTIMBREAMOUNT*6+1;
			uint8_t mult_data[data_sz];

			for(int i = 0; i < MULTTIMBREAMOUNT; i++)
			{
				mult_data[i] = main_mult.mtelement[i].en;
				mult_data[i+MULTTIMBREAMOUNT]   = main_mult.mtelement[i].destination;
				mult_data[i+2*MULTTIMBREAMOUNT] = main_mult.mtelement[i].parameters[MULTSTARTPos];
				mult_data[i+3*MULTTIMBREAMOUNT] = main_mult.mtelement[i].parameters[MULTENDPos];
				mult_data[i+4*MULTTIMBREAMOUNT] = main_mult.mtelement[i].parameters[MULTRANGESHIFTPos];
				mult_data[i+5*MULTTIMBREAMOUNT] = main_mult.mtelement[i].open;
			}
			mult_data[data_sz - 1] = main_mult.active_mtindex;

			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVEMULTPG, mult_data, data_sz);


		}

		note_range_print(&main_mult.mtelement[main_mult.active_mtindex]);//debug
		number_to_serial_human_readable(main_mult.active_mtindex);       //debug
		//working_mode = PLAYMODE;
	}
}
void mtelement_mute_toggle(int index)
{
	real_mtelement_mute_toggle(&main_mult, &main_mult.mtelement[index], index);
}

/////////
////TRANS
void prev_trelement()
{
	real_prev_trelement(&main_trans);
}
void next_trelement()
{
	real_next_trelement(&main_trans);
}
void prev_trparam()
{
	real_prev_trparam(&main_trans.trelement[main_trans.active_trindex]);
}
void next_trparam()
{
	real_next_trparam(&main_trans.trelement[main_trans.active_trindex]);
}


void note_trans_learn_mode_toggle()
{
	if(working_mode == PLAYMODE)
	{
		trans_start_of_learn(&main_trans.trelement[main_trans.active_trindex]);
		//working_mode = LEARNMODE;
	}
	else
	{
		trans_end_of_learn(&main_trans.trelement[main_trans.active_trindex]);

		if((GPIOC->IDR & 1 << CFGBTNPIN) == 0)
		{
			uint8_t data_sz = TRANSCCAMOUNT*5 + 1;
			uint8_t trans_data[data_sz];

			for(int i = 0; i < TRANSCCAMOUNT; i++)
			{
				trans_data[i] 					= main_trans.trelement[i].en;
				trans_data[i+TRANSCCAMOUNT]   	= main_trans.trelement[i].parameters[TRANSCCPos];
				trans_data[i+2*TRANSCCAMOUNT] 	= main_trans.trelement[i].parameters[TRANSSRCPos];
				trans_data[i+3*TRANSCCAMOUNT] 	= main_trans.trelement[i].parameters[TRANSDSTPos];
				trans_data[i+4*TRANSCCAMOUNT] 	= main_trans.trelement[i].open;
			}
			trans_data[data_sz - 1] = main_trans.active_trindex;

			at45dblib_wait_ready_bit();
			at45dblib_page_prog_1(SAVETRANSPG, trans_data, data_sz);

		}

		note_trans_print(&main_trans.trelement[main_trans.active_trindex]);//debug
		number_to_serial_human_readable(main_trans.active_trindex);       //debug
		//working_mode = PLAYMODE;
	}
}

void trelement_mute_toggle(int index)
{
	real_trelement_mute_toggle(&main_trans, &main_trans.trelement[index], index);
}

/////////////////////////
/////////////////////////
////BYTES & MIDI Messages
void byte_receive_from_usart3()
{
	rx_byte =  USART3->DR;
	if(waiting_bytes_push_index < MIDIQUEUEN-2)
	{
		rx_byte_waiting_list[waiting_bytes_push_index++] = rx_byte;
//		if(rx_byte == 0xF8)
//			byte_to_midi_device(rx_byte);
	}

}

void byte_receive_from_usart2()
{
	rx_byte2 =  USART2->DR;
//	rx_byte_waiting_list[waiting_bytes_push_index++] = rx_byte;
	if(waiting_bytes_push_index2 < MIDIQUEUEN-2)
	{
		rx_byte_waiting_list2[waiting_bytes_push_index2++] = rx_byte2;
//		if(rx_byte2 == 0xF8)
//			byte_to_midi_device(rx_byte2);
	}
}

void  message_assambling()
{
	if(waiting_bytes_push_index != 0 && waiting_bytes_pop_index  != waiting_bytes_push_index)//not all received bytes proceeded
	{
		find_place_in_midi_msg(rx_byte_waiting_list[waiting_bytes_pop_index]);
		rx_byte_waiting_list[waiting_bytes_pop_index++] = 0;
	}
	else if(waiting_bytes_push_index != 0 && waiting_bytes_pop_index == waiting_bytes_push_index)
	{
		waiting_bytes_push_index = 0;
		waiting_bytes_pop_index  = 0;
		//  если при этом в сплитах остались активные значения,
		//  их надо сбросить и отправить в эфир all note off
	}
}

void message_assambling2()
{
	if(waiting_bytes_push_index2 != 0 && waiting_bytes_pop_index2  != waiting_bytes_push_index2)//not all received bytes proceeded
	{
		find_place_in_midi_msg2(rx_byte_waiting_list2[waiting_bytes_pop_index2]);
		rx_byte_waiting_list2[waiting_bytes_pop_index2++] = 0;
	}
	else if(waiting_bytes_push_index2 != 0 && waiting_bytes_pop_index2 == waiting_bytes_push_index2)
	{
		waiting_bytes_push_index2 = 0;
		waiting_bytes_pop_index2  = 0;
		//  если при этом в сплитах остались активные значения,
		//  их надо сбросить и отправить в эфир all note off
	}
}

void find_place_in_midi_msg(uint8_t t_byte)//����������� ���� � �������� ��� � ������ ��� ���� ����
{
//	number_to_serial_human_readable(t_byte);	//debug
//	word_to_serial("\r");						//debug


	if((t_byte >> 4) > 7)//status byte
	{
//		midi_data_flag = 0;
		if((t_byte >> 4) == 0x9 || (t_byte >> 4) == 0x8) //noteOn or noteOff
		{
			midi_data_flag = 0;
			status_byte = t_byte;
			//incoming_note.status = status_byte;
			msg_type = GOTNOTE;
		}
		else if(t_byte == 0xFA)//MIDI START
		{

			midiclk_counter = 0;
			midiclk_state_counter = MIDICLKONSTATE;

			///// loading of MIDI CLK variables
			// use func ptrarr
			uint8_t t_psc, t_clk_pol, t_flt_sys;
			if(global_mode == SEQMODE)
			{
				t_psc     = main_cfg.seq[MIDICLKPSCPos];
				t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
				t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == POLYMODE)
			{
				t_psc     = main_cfg.stack[MIDICLKPSCPos];
				t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
				t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == MULTMODE)
			{
				t_psc     = main_cfg.mult[MIDICLKPSCPos];
				t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
				t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == TRANSMODE)
			{
				t_psc     = main_cfg.trans[MIDICLKPSCPos];
				t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
				t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
			}
			////////////////////////////////////////////////////////

			if(t_clk_pol == MIDInCLKPOL)
			{
				CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);				//MIDI Sync Pin Reset
			}
			else if (t_clk_pol == MIDICLKPOL)
			{
				CLKLEDPORT->ODR |= 1 << CLKLEDPIN;					//MIDI Sync Pin Set
			}

			if(t_flt_sys == MIDITRANSPORTOPEN)
			{
				byte_to_midi_device(t_byte);
			}
			//пока не пересылается в выход
			//msg_type = GOTOTHER;
		}
		else if (t_byte == 0xFC) // MIDI STOP
		{
			//byte_to_midi_device(t_byte);
			midiclk_counter = 0xFF;

			//Cброс всех индексов всех секвенсоров
			if(global_mode == SEQMODE)
				reset_all_seq_ind();

			///// loading of MIDI CLK variables
			// use func ptrarr
			uint8_t t_psc, t_clk_pol, t_flt_sys;
			if(global_mode == SEQMODE)
			{
				t_psc     = main_cfg.seq[MIDICLKPSCPos];
				t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
				t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == POLYMODE)
			{
				t_psc     = main_cfg.stack[MIDICLKPSCPos];
				t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
				t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == MULTMODE)
			{
				t_psc     = main_cfg.mult[MIDICLKPSCPos];
				t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
				t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == TRANSMODE)
			{
				t_psc     = main_cfg.trans[MIDICLKPSCPos];
				t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
				t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
			}
			////////////////////////////////////////////////////////

			if(t_flt_sys == MIDITRANSPORTOPEN)
			{
				byte_to_midi_device(t_byte);
			}
			// отправить всем нотОнам по нотофу. Или вообще отправить all notes off
		}
		else if((t_byte >> 4) == 0xB) //any CC
		{
			/*
			number_to_serial_human_readable(t_byte);	//debug
			word_to_serial(" - CC\r");					//debug
			*/

			midi_data_flag = 0;
			incoming_CC_msg.status = t_byte;
			msg_type = GOTCC;
		}
		else if((t_byte >> 4) == 0xA) //Aftertouch
		{
			midi_data_flag = 0;
			incoming_CC_msg.status = t_byte;
			msg_type = GOTAFTERTOUCH;
		}
		else if((t_byte >> 4) == 0xE) //Pitch Bend
		{
			midi_data_flag = 0;
			incoming_CC_msg.status = t_byte;
		    msg_type = GOTPITCHBEND;

			//msg_type = GOTOTHER;
			//midi THRU
			//byte_to_midi_device(t_byte);

		}
		else if((t_byte >> 4) == 0xD) //Channel Aftertoch
		{
			midi_data_flag = 0;
			incoming_CC_msg.status = t_byte;
		    msg_type = GOTCHAFTTOUCH;

			//msg_type = GOTOTHER;
			//midi THRU
			//byte_to_midi_device(t_byte);

		}
		else if(t_byte == 0xF8)//MIDI CLK
		{
//			byte_to_midi_device(t_byte);

				///// loading of MIDI CLK variables
				uint8_t t_psc, t_clk_pol, t_flt_sys;
				if(global_mode == SEQMODE)
				{
					t_psc     = main_cfg.seq[MIDICLKPSCPos];
					t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
					t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == POLYMODE)
				{
					t_psc     = main_cfg.stack[MIDICLKPSCPos];
					t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
					t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == MULTMODE)
				{
					t_psc     = main_cfg.mult[MIDICLKPSCPos];
					t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
					t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == TRANSMODE)
				{
					t_psc     = main_cfg.trans[MIDICLKPSCPos];
					t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
					t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
				}
				////////////////////////////////////////////////////////


				if(midiclk_counter == t_psc) 		// Counter has reached prescaler value
				{
					midiclk_counter = 0;
					/////// Control of CLK PIN state
					midiclk_state_counter = MIDICLKONSTATE;
					if(t_clk_pol == MIDInCLKPOL)
					{
						CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);				//MIDI Sync Pin Reset
					}
					else if (t_clk_pol == MIDICLKPOL)
					{
						CLKLEDPORT->ODR |= 1 << CLKLEDPIN;					//MIDI Sync Pin Set
					}
					///////
				}
				else if(midiclk_counter < t_psc)	// Counter is rising
				{
					midiclk_counter++;
				}
				else if(midiclk_counter > t_psc)	// Somehow Counter is overflow prescaler value
				{
					midiclk_counter = 0;
				}

//			if(midiclk_counter != MIDICLKCNTSTOP) 	// Counter blocked by Stop Message
//			{
//				///// loading of MIDI CLK variables
//				uint8_t t_psc, t_clk_pol, t_flt_sys;
//				if(global_mode == SEQMODE)
//				{
//					t_psc     = main_cfg.seq[MIDICLKPSCPos];
//					t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
//					t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
//				}
//				else if(global_mode == POLYMODE)
//				{
//					t_psc     = main_cfg.stack[MIDICLKPSCPos];
//					t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
//					t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
//				}
//				else if(global_mode == MULTMODE)
//				{
//					t_psc     = main_cfg.mult[MIDICLKPSCPos];
//					t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
//					t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
//				}
//				else if(global_mode == TRANSMODE)
//				{
//					t_psc     = main_cfg.trans[MIDICLKPSCPos];
//					t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
//					t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
//				}
//				////////////////////////////////////////////////////////
//
//
//				if(midiclk_counter == t_psc) 		// Counter has reached prescaler value
//				{
//					midiclk_counter = 0;
//					/////// Control of CLK PIN state
//					midiclk_state_counter = MIDICLKONSTATE;
//					if(t_clk_pol == MIDInCLKPOL)
//					{
//						CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);				//MIDI Sync Pin Reset
//					}
//					else if (t_clk_pol == MIDICLKPOL)
//					{
//						CLKLEDPORT->ODR |= 1 << CLKLEDPIN;					//MIDI Sync Pin Set
//					}
//					///////
//				}
//				else if(midiclk_counter < t_psc)	// Counter is rising
//				{
//					midiclk_counter++;
//				}
//				else if(midiclk_counter > t_psc)	// Somehow Counter is overflow prescaler value
//				{
//					midiclk_counter = 0;
//				}
//			}


			byte_to_midi_device(t_byte);

		}
		else
		{
			msg_type = GOTOTHER;
			//midi THRU
			byte_to_midi_device(t_byte);
//			number_to_serial_human_readable(t_byte);	//debug
//			word_to_serial("\r");						//debug
		}
	}
	else  												//data byte
	{
		if(msg_type == GOTNOTE)
		{
			if(midi_data_flag == 0)
			{
				incoming_note.data1 = t_byte;
				midi_data_flag = 1;
			}
			else
			{
				incoming_note.data2 = t_byte;
				midi_data_flag = 0;
				if(t_byte == 0) // velocity - 0
				{
					//uint8_t temp = incoming_note.status;
					uint8_t temp = status_byte;
					temp = (temp & 0xF) + 0x80;
					incoming_note.status = temp;
				}
				else
					incoming_note.status = status_byte;
				incoming_note.complete_flag = 1;
			}

		}
		else if(msg_type == GOTCC)
		{
			if(midi_data_flag == 0)
			{
				incoming_CC_msg.data1 = t_byte;
				midi_data_flag = 1;
			}
			else
			{
				incoming_CC_msg.data2 = t_byte;
				midi_data_flag = 0;
				incoming_CC_msg.complete_flag = 1;
			}
		}
		else if(msg_type == GOTAFTERTOUCH)
		{
			if(midi_data_flag == 0)
			{
				incoming_CC_msg.data1 = t_byte;
				midi_data_flag = 1;
			}
			else
			{
				incoming_CC_msg.data2 = t_byte;
				midi_data_flag = 0;
				incoming_CC_msg.complete_flag = 1;
			}

		}
		else if(msg_type == GOTCHAFTTOUCH)
		{
			incoming_CC_msg.data1 = t_byte;
			incoming_CC_msg.complete_flag = 1;
		}
		else if(msg_type == GOTPITCHBEND)
		{
			if(midi_data_flag == 0)
			{
				incoming_CC_msg.data1 = t_byte;
				midi_data_flag = 1;
			}
			else
			{
				incoming_CC_msg.data2 = t_byte;
				midi_data_flag = 0;
				incoming_CC_msg.complete_flag = 1;
			}
		}
		else if(msg_type == GOTOTHER)
		{
			//skip, but trully need just send thru
			byte_to_midi_device(t_byte);
		}
	}

}

void find_place_in_midi_msg2(uint8_t t_byte)//����������� ���� � �������� ��� � ������ ��� ���� ����
{
	//number_to_serial_human_readable(t_byte);	//debug
	//word_to_serial("\r");						//debug

	if((t_byte >> 4) > 7)//status byte
	{
//		midi_data_flag2 = 0;
		if((t_byte >> 4) == 0x9 || (t_byte >> 4) == 0x8) //noteOn or noteOff
		{
			//incoming_note2.status = t_byte;
			midi_data_flag2 = 0;
			status_byte2 = t_byte;
			msg_type2 = GOTNOTE;
		}
		else if(t_byte == 0xF8)//MIDI CLK
		{
			/*
			if(t_flt_sys == 0)
			{
				byte_to_midi_device(t_byte);
			}
			*/
			if(midiclk_counter != MIDICLKCNTSTOP) 	// Counter blocked by Stop Message
			{
				/////loading of MIDI CLK variables
				///// func ptr arr
				uint8_t t_psc, t_clk_pol, t_flt_sys;
				if(global_mode == SEQMODE)
				{
					t_psc     = main_cfg.seq[MIDICLKPSCPos];
					t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
					t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == POLYMODE)
				{
					t_psc     = main_cfg.stack[MIDICLKPSCPos];
					t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
					t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == MULTMODE)
				{
					t_psc     = main_cfg.mult[MIDICLKPSCPos];
					t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
					t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
				}
				else if(global_mode == TRANSMODE)
				{
					t_psc     = main_cfg.trans[MIDICLKPSCPos];
					t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
					t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
				}
				///////////////////////////////////////////////////////


				if(midiclk_counter == t_psc) 		// Counter has reached prescaler value
				{
					midiclk_counter = 0;
					/////// Control of CLK PIN state
					midiclk_state_counter = MIDICLKONSTATE;
					if(t_clk_pol == MIDInCLKPOL)
					{
						CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);				//MIDI Sync Pin Reset
					}
					else if (t_clk_pol == MIDICLKPOL)
					{
						CLKLEDPORT->ODR |= 1 << CLKLEDPIN;					//MIDI Sync Pin Set
					}
					///////
				}
				else if(midiclk_counter < t_psc)	// Counter is rising
				{
					midiclk_counter++;
				}
				else if(midiclk_counter > t_psc)	// Somehow Counter is overflow prescaler value
				{
					midiclk_counter = 0;
				}
			}
			byte_to_midi_device(t_byte);

			//проверить почему сторока снизу закоментирована <<  вроде норм,
			//так как это сообщение сразу высылается в эфир, все должно работать
			//msg_type = GOTOTHER;

			//CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);

		}
		else if(t_byte == 0xFA)//MIDI START
		{
			midiclk_counter = 0;
			midiclk_state_counter = MIDICLKONSTATE;

			/////loading of MIDI CLK variables
			///// func ptr arr
			uint8_t t_psc, t_clk_pol, t_flt_sys;
			if(global_mode == SEQMODE)
			{
				t_psc     = main_cfg.seq[MIDICLKPSCPos];
				t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
				t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == POLYMODE)
			{
				t_psc     = main_cfg.stack[MIDICLKPSCPos];
				t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
				t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == MULTMODE)
			{
				t_psc     = main_cfg.mult[MIDICLKPSCPos];
				t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
				t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == TRANSMODE)
			{
				t_psc     = main_cfg.trans[MIDICLKPSCPos];
				t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
				t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
			}
			///////////////////////////////////////////////////////

			if(t_clk_pol == MIDInCLKPOL)
			{
				CLKLEDPORT->ODR &= ~(1 << CLKLEDPIN);				//MIDI Sync Pin Reset
			}
			else if (t_clk_pol == MIDICLKPOL)
			{
				CLKLEDPORT->ODR |= 1 << CLKLEDPIN;					//MIDI Sync Pin Set
			}

			if(t_flt_sys == MIDITRANSPORTOPEN)
			{
				byte_to_midi_device(t_byte);
			}
			//пока не пересылается в выход
			//msg_type = GOTOTHER;
		}
		else if (t_byte == 0xFC) // MIDI STOP
		{
			midiclk_counter = 0xFF;


			/////loading of MIDI CLK variables
			///// func ptr arr
			uint8_t t_psc, t_clk_pol, t_flt_sys;
			if(global_mode == SEQMODE)
			{
				t_psc     = main_cfg.seq[MIDICLKPSCPos];
				t_clk_pol = main_cfg.seq[MIDICLKPOLPos];
				t_flt_sys = main_cfg.seq[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == POLYMODE)
			{
				t_psc     = main_cfg.stack[MIDICLKPSCPos];
				t_clk_pol = main_cfg.stack[MIDICLKPOLPos];
				t_flt_sys = main_cfg.stack[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == MULTMODE)
			{
				t_psc     = main_cfg.mult[MIDICLKPSCPos];
				t_clk_pol = main_cfg.mult[MIDICLKPOLPos];
				t_flt_sys = main_cfg.mult[MIDITRANSPORTFLTPos];
			}
			else if(global_mode == TRANSMODE)
			{
				t_psc     = main_cfg.trans[MIDICLKPSCPos];
				t_clk_pol = main_cfg.trans[MIDICLKPOLPos];
				t_flt_sys = main_cfg.trans[MIDITRANSPORTFLTPos];
			}
			///////////////////////////////////////////////////////

			//Cброс всех индексов всех секвенсоров
			if(global_mode == SEQMODE)
				reset_all_seq_ind();
			if(t_flt_sys == MIDITRANSPORTOPEN)
			{
				byte_to_midi_device(t_byte);
			}
			// отправить всем нотОнам по нотофу. Или вообще отправить all notes off
		}

		else if((t_byte >> 4) == 0xB) //any CC
		{
			/*
			number_to_serial_human_readable(t_byte);	//debug
			word_to_serial(" - CC\r");					//debug
			*/
			midi_data_flag2 = 0;
			incoming_CC_msg2.status = t_byte;
			msg_type2 = GOTCC;
		}
		else if((t_byte >> 4) == 0xA) //Aftertouch
		{
			midi_data_flag2 = 0;
			incoming_CC_msg2.status = t_byte;
			msg_type2 = GOTAFTERTOUCH;
		}
		else if((t_byte >> 4) == 0xE) //Pitch Bend
		{
			midi_data_flag2 = 0;
			incoming_CC_msg2.status = t_byte;
		    msg_type2 = GOTPITCHBEND;

			//msg_type = GOTOTHER;
			//midi THRU
			//byte_to_midi_device(t_byte);

		}
		else if((t_byte >> 4) == 0xD) //Channel Aftertoch
		{
			midi_data_flag2 = 0;
			incoming_CC_msg2.status = t_byte;
		    msg_type2 = GOTCHAFTTOUCH;

			//msg_type = GOTOTHER;
			//midi THRU
			//byte_to_midi_device(t_byte);

		}
		else
		{
			msg_type2 = GOTOTHER;
			//midi THRU
			byte_to_midi_device(t_byte);
//			number_to_serial_human_readable(t_byte);	//debug
//			word_to_serial("\r");						//debug
		}
	}
	else  												//data byte
	{
		if(msg_type2 == GOTNOTE)
		{
			if(midi_data_flag2 == 0)
			{
				incoming_note2.data1 = t_byte;
				midi_data_flag2 = 1;
			}
			else
			{
//				incoming_note2.data2 = t_byte;
//				midi_data_flag2 = 0;
//				if(t_byte == 0)
//				{
//					uint8_t temp = incoming_note2.status;
//					temp = (temp & 0xF) + 0x80;
//					incoming_note2.status = temp;
//				}
//				incoming_note2.complete_flag = 1;
//
//
				/////////////////////////

				incoming_note2.data2 = t_byte;
				midi_data_flag2 = 0;
				if(t_byte == 0) // velocity - 0
				{
					uint8_t temp = status_byte2;
					temp = (temp & 0xF) + 0x80;
					incoming_note2.status = temp;
				}
				else
					incoming_note2.status = status_byte2;
				incoming_note2.complete_flag = 1;
			}

		}
		else if(msg_type2 == GOTCC)
		{
			if(midi_data_flag2 == 0)
			{
				incoming_CC_msg2.data1 = t_byte;
				midi_data_flag2 = 1;
			}
			else
			{
				incoming_CC_msg2.data2 = t_byte;
				midi_data_flag2 = 0;
				incoming_CC_msg2.complete_flag = 1;
			}
		}
		else if(msg_type2 == GOTAFTERTOUCH)
		{
			if(midi_data_flag2 == 0)
			{
				incoming_CC_msg2.data1 = t_byte;
				midi_data_flag2 = 1;
			}
			else
			{
				incoming_CC_msg2.data2 = t_byte;
				midi_data_flag2 = 0;
				incoming_CC_msg2.complete_flag = 1;
			}
		}
		else if(msg_type2 == GOTCHAFTTOUCH)
		{
				incoming_CC_msg2.data1 = t_byte;
				incoming_CC_msg2.complete_flag = 1;
		}
		else if(msg_type2 == GOTPITCHBEND)
		{
			if(midi_data_flag2 == 0)
			{
				incoming_CC_msg2.data1 = t_byte;
				midi_data_flag2 = 1;
			}
			else
			{
				incoming_CC_msg2.data2 = t_byte;
				midi_data_flag2 = 0;
				incoming_CC_msg2.complete_flag = 1;
			}
		}
		else if(msg_type2 == GOTOTHER)
		{
			//skip, but trully need just send thru
			byte_to_midi_device(t_byte);
		}
	}

}



void incoming_CC_processing(int in)
{
	midi_msg_init(&incoming_CC_msg_buffer);

	if(in == 1)
	{
		incoming_CC_msg_buffer = incoming_CC_msg;
		incoming_CC_msg.data1 = 0;
		incoming_CC_msg.data2 = 0;
		incoming_CC_msg.complete_flag = 0;
		incoming_CC_msg.trig = 0;
	}
	else if(in == 2)
	{
		incoming_CC_msg_buffer = incoming_CC_msg2;
		incoming_CC_msg2.data1 = 0;
		incoming_CC_msg2.data2 = 0;
		incoming_CC_msg2.complete_flag = 0;
		incoming_CC_msg2.trig = 0;
	}


	if((incoming_CC_msg_buffer.status & 0xF0) == 0xB0) // got CC
	{
		if(incoming_CC_msg_buffer.data1 < 122)
		{
			if     (incoming_CC_msg_buffer.data1 == comm_CC_knob)  //communication CC
			{
				comm_knob_CC_processing();
			}
			else if(incoming_CC_msg_buffer.data1 != comm_CC_knob) // not communication, not AllnoteOff
			{
				normal_CC_processing();
			}
		}
		else if(incoming_CC_msg_buffer.data1 >= 122)
		{
			midi_msg_to_synth(&incoming_CC_msg_buffer); // All NoteOff

			//midiclk_counter = 0xFF;

			//word_to_serial("CC larg:");//debug
			//midi_msg_print(&incoming_CC_msg_buffer); //debug
		}
	}
	else if((incoming_CC_msg_buffer.status & 0xF0) == 0xE0) // got Pitch Bend
	{
		if(global_mode == TRANSMODE)
		{
			send_CC_to_not_open_trans();
			//midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == POLYMODE)
		{
			//питчбэнд входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos])  //from input channel
			{
				send_CC_to_active_destinations();
			}
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == MULTMODE)
		{
			//питчбэнд входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos])  //from input channel
			{
				send_CC_to_active_ranges();
			}
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else
		{
			//а для контейнера отправить это сообщение в ту же очередь, куда полетят фру сообщения
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
//		midi_msg_to_synth(&incoming_CC_msg_buffer);
	}
	else if((incoming_CC_msg_buffer.status & 0xF0) == 0xA0) //got Aftertouch
	{
		if(global_mode == TRANSMODE)
		{
			// send_CC_to_not_open_trans()
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == POLYMODE)
		{
			//афтертач входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos])  //from input channel
			{
				send_CC_to_active_destinations();
			}
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == MULTMODE)
		{
			//афтертач входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos])  //from input channel
			{
				send_CC_to_active_ranges();
			}
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else
		{
			//а для контейнера отправить это сообщение в ту же очередь, куда полетят фру сообщения
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}

//		midi_msg_to_synth(&incoming_CC_msg_buffer);
	}
	else if((incoming_CC_msg_buffer.status & 0xF0) == 0xD0) //got Channel Aftertouch
	{
		if(global_mode == TRANSMODE)
		{
			send_CC_to_not_open_trans();
			//midi_chanafttouch_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == POLYMODE)
		{

			//афтертач входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos])  //from input channel
			{
				send_CC_to_active_destinations();
			}

			midi_chanafttouch_to_synth(&incoming_CC_msg_buffer);
		}
		else if(global_mode == MULTMODE)
		{

			//афтертач входного канала, который будет отправлен во все дестинейшены
			if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos])  //from input channel
			{
				send_CC_to_active_ranges();
			}

			midi_chanafttouch_to_synth(&incoming_CC_msg_buffer);
		}
		else
		{
			//а для контейнера отправить это сообщение в ту же очередь, куда полетят фру сообщения
			midi_chanafttouch_to_synth(&incoming_CC_msg_buffer);
		}

//		midi_msg_to_synth(&incoming_CC_msg_buffer);
	}


/*
	word_to_serial("Thru:");//debug
	midi_msg_print(&incoming_CC_msg_buffer); //debug
*/
}


void comm_knob_CC_processing()
{
	if(global_mode == SEQMODE)
	{
		if(sequencer_arr[active_sequencer_index].learn == LEARNMODE && delete_process_flag == 0 && pause_dead_time_counter == 0)
		{
			pause_dead_time_counter = PAUSEDEADTIME;
			pause_append_to_sequencer(&sequencer_arr[active_sequencer_index]); //� ���� ����������� �����
		}
		else if(working_mode == CFGMODE)
		{
			// MIDI/DAC Switch Config
			uint8_t t_i = main_cfg.active_index;
			uint8_t t_d = incoming_CC_msg_buffer.data2 >> 3;
			main_cfg.seq[t_i] = t_d;
			midi_dac_switch_cfg(t_i, t_d);
			/*
			if(t_i == MIDIOUTTHRUPos)
			{
				if(t_d == DACSYNCSW)
				{
					GPIOB->ODR |=   1<<12; // CTRL B-D = 1 -> DAC
					GPIOB->ODR &= ~(1<<14);// CTRL A-C = 0 -> nMIDI
				}
				else
				{
					GPIOB->ODR &= ~(1<<12); // CTRL B-D = 0 -> nDAC
					GPIOB->ODR |=  (1<<14); // CTRL A-C = 1 -> MIDI
				}
			}
			*/
		}
		else if(working_mode == PLAYMODE)
		{
			//MIDI CC Thru
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
	}
	else if(global_mode == POLYMODE)
	{
		if(main_stack.stelement[main_stack.active_stindex].learn == LEARNMODE)
		{
			main_stack.stelement[main_stack.active_stindex].destination = incoming_CC_msg_buffer.data2 >> 3; //take channel number
			main_stack.stelement[main_stack.active_stindex].en = 1; //now this element is enabled

			//word_to_serial("DST ");//debug
			//note_stack_element_print(&main_stack.stelement[main_stack.active_stindex]);//debug
		}
		else if(working_mode == CFGMODE)
		{
//			main_cfg.stack[main_cfg.active_index] = incoming_CC_msg_buffer.data2 >> 3;

			// MIDI/DAC Switch Config
			uint8_t t_i = main_cfg.active_index;
			uint8_t t_d = incoming_CC_msg_buffer.data2 >> 3;
			main_cfg.stack[t_i] = t_d;

			midi_dac_switch_cfg(t_i, t_d);

//			if(t_i == MIDIOUTTHRUPos)
//			{
//				if(t_d == DACSYNCSW)
//				{
//					GPIOB->ODR |=   1<<12; // CTRL B-D = 1 -> DAC
//					GPIOB->ODR &= ~(1<<14);// CTRL A-C = 0 -> nMIDI
//				}
//				else
//				{
//					GPIOB->ODR &= ~(1<<12); // CTRL B-D = 0 -> nDAC
//					GPIOB->ODR |=  (1<<14); // CTRL A-C = 1 -> MIDI
//				}
//			}

			//midi thru
			//midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(working_mode == PLAYMODE)
		{
			//MIDI CC Thru
			//МОДВИЛ входного канала, который будет отправлен во все дестинейшены
			if((   incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos]  //from input channel
                && incoming_CC_msg_buffer.data1 		== 1)							//modwheel
			{
				send_CC_to_active_destinations();
			}

			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
	}
	else if(global_mode == MULTMODE)
	{
//		if(main_mult.mtelement[main_mult.active_mtindex].learn == LEARNMODE)
		if(working_mode == LEARNMODE)
		{
			main_mult.mtelement[main_mult.active_mtindex].parameters[MULTRANGESHIFTPos] = incoming_CC_msg_buffer.data2 >> 3; //take data
			//main_mult.mtelement[main_mult.active_mtindex].en = 1; //now this note range is enabled

//			word_to_serial("range DST: ");//debug
//			note_range_print(&main_mult.mtelement[main_mult.active_mtindex]);//debug
		}
		else if(working_mode == CFGMODE)
		{
//			main_cfg.mult[main_cfg.active_index] = incoming_CC_msg_buffer.data2 >> 3;

			// MIDI/DAC Switch Config
			uint8_t t_i = main_cfg.active_index;
			uint8_t t_d = incoming_CC_msg_buffer.data2 >> 3;
			main_cfg.mult[t_i] = t_d;

			midi_dac_switch_cfg(t_i, t_d);

//			if(t_i == MIDIOUTTHRUPos)
//			{
//				if(t_d == DACSYNCSW)
//				{
//					GPIOB->ODR |=   1<<12; // CTRL B-D = 1 -> DAC
//					GPIOB->ODR &= ~(1<<14);// CTRL A-C = 0 -> nMIDI
//				}
//				else
//				{
//					GPIOB->ODR &= ~(1<<12); // CTRL B-D = 0 -> nDAC
//					GPIOB->ODR |=  (1<<14); // CTRL A-C = 1 -> MIDI
//				}
//			}
			//midi thru
			//midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else if(working_mode == PLAYMODE)
		{
			//MIDI CC Thru
			//Модвил со входного канала отправить во все активные дестинейшены
			if((   incoming_CC_msg_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos]  //from input channel
                && incoming_CC_msg_buffer.data1 		== 1)							//modwheel
			{
				send_CC_to_active_ranges();
			}
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
	}
	else if(global_mode == TRANSMODE)
	{
		if(working_mode == LEARNMODE)
		{
			uint8_t learn_index;
			learn_index = main_trans.trelement[main_trans.active_trindex].active_trparamindex;
			if(learn_index != TRANSCCPos)
			{
				main_trans.trelement[main_trans.active_trindex].parameters[learn_index] = incoming_CC_msg_buffer.data2 >> 3; //take CC value into channel
			}
			else
			{
				main_trans.trelement[main_trans.active_trindex].parameters[learn_index] = incoming_CC_msg_buffer.data2;  //take CC value into CC number
				main_trans.trelement[main_trans.active_trindex].en = 1; //now  cell is enabled
			}

			//word_to_serial("trans param: ");//debug
			//note_trans_print(&main_trans.trelement[main_trans.active_trindex]);//debug
		}
		else if(working_mode == CFGMODE)
		{
			//main_cfg.trans[main_cfg.active_index] = incoming_CC_msg_buffer.data2 >> 3;

			// MIDI/DAC Switch Config
			uint8_t t_i = main_cfg.active_index;
			uint8_t t_d = incoming_CC_msg_buffer.data2 >> 3;
			main_cfg.trans[t_i] = t_d;

			midi_dac_switch_cfg(t_i, t_d);

//			if(t_i == MIDIOUTTHRUPos)
//			{
//				if(t_d == DACSYNCSW)
//				{
//					GPIOB->ODR |=   1<<12; // CTRL B-D = 1 -> DAC
//					GPIOB->ODR &= ~(1<<14);// CTRL A-C = 0 -> nMIDI
//				}
//				else
//				{
//					GPIOB->ODR &= ~(1<<12); // CTRL B-D = 0 -> nDAC
//					GPIOB->ODR |=  (1<<14); // CTRL A-C = 1 -> MIDI
//				}
//			}
			//midi thru
			//midi_msg_to_synth(&incoming_CC_msg_buffer); //этого здесь быть не должно
		}
		else if(working_mode == PLAYMODE)
		{
			//MIDI CC Thru
			// здесь ничего менять не нужно модвил и так должен пролететь куда надо
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}

	}
	else
	{
		midi_msg_to_synth(&incoming_CC_msg_buffer);
	}

}



void normal_CC_processing()
{
	if(global_mode == TRANSMODE)
	{
		if(main_trans.trelement[main_trans.active_trindex].learn == LEARNMODE)
		{
			// CC Channel become trans dest channel
			main_trans.trelement[main_trans.active_trindex].parameters[TRANSDSTPos] = incoming_CC_msg_buffer.status & 0xF; //take channel number
			main_trans.trelement[main_trans.active_trindex].parameters[TRANSCCPos]  = incoming_CC_msg_buffer.data1; //take CC number
			main_trans.trelement[main_trans.active_trindex].en = 1; //now  cell is enabled
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		else
		{
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
	}
	else if(global_mode == POLYMODE)
	{
		if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos]) // from input channel
		{
			send_CC_to_active_destinations();
		}
		else
		{
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}
		/*
		//МОДВИЛ входного канала, который будет отправлен во все дестинейшены
		if((   incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos]  //from input channel
            && incoming_CC_msg_buffer.data1 		== 1)							//modwheel
		{
			send_CC_to_active_destinations();
		}
		midi_msg_to_synth(&incoming_CC_msg_buffer);
		*/
	}
	else if(global_mode == MULTMODE)
	{
		//МОДВИЛ входного канала, который будет отправлен во все дестинейшены
		if((incoming_CC_msg_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos])  //from input channel
		{
			send_CC_to_active_ranges();
		}
		else
		{
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}

		/*
		//МОДВИЛ входного канала, который будет отправлен во все дестинейшены
		if((   incoming_CC_msg_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos]  //from input channel
            && incoming_CC_msg_buffer.data1 		== 1)							//modwheel
		{
			send_CC_to_active_ranges();
		}
		midi_msg_to_synth(&incoming_CC_msg_buffer);
		*/
	}
	else
	{
		//тут тоже нужно для мульта и комбайнера добавить варианты
		//а для контейнера отправить это сообщение в ту же очередь, куда полетят фру сообщения
		midi_msg_to_synth(&incoming_CC_msg_buffer); // это для всех режимов в том числе и пустых
	}
}



void send_CC_to_active_destinations()
{
	struct midi_msg t_msg;
	t_msg = incoming_CC_msg_buffer;
	uint8_t t_stat = t_msg.status & 0xF0;

	if(msg_type == GOTCHAFTTOUCH)
	{
		for(int i = 0; i < STACKHEIGHT; i++)
		{
			if(main_stack.stelement[i].active != 0)
			{
//				t_msg.status &= 0xF0;
//				t_msg.status += main_stack.stelement[i].destination;
				t_msg.status = t_stat + main_stack.stelement[i].destination;
				midi_chanafttouch_to_synth(&t_msg);
			}
		}
	}
	else
	{
		for(int i = 0; i < STACKHEIGHT; i++)
		{
			if(main_stack.stelement[i].active != 0)
			{
				//t_msg.status &= 0xF0;
				//t_msg.status += main_stack.stelement[i].destination;
				t_msg.status = t_stat + main_stack.stelement[i].destination;
				midi_msg_to_synth(&t_msg);
			}
		}
	}
}


void send_CC_to_active_ranges()
{
	struct midi_msg t_msg;
	//midi_msg_init(&t_msg);

// new
	t_msg = incoming_CC_msg_buffer;
	uint8_t t_stat = t_msg.status & 0xF0;
	if(msg_type == GOTCHAFTTOUCH)
	{
		for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		{
			if(main_mult.mtelement[i].active != 0 && main_mult.mtelement[i].en != 0)
			{
//				t_msg = incoming_CC_msg_buffer;
				//t_msg.status &= 0xF0;
				t_msg.status = t_stat + main_mult.mtelement[i].destination;

				midi_chanafttouch_to_synth(&t_msg);
			}
		}
	}
	else
	{
		for(int i = 0; i < MULTTIMBREAMOUNT; i++)
		{
			if(main_mult.mtelement[i].active != 0 && main_mult.mtelement[i].en != 0)
			{
//				t_msg = incoming_CC_msg_buffer;
//				t_msg.status &= 0xF0;
				t_msg.status = t_stat+ main_mult.mtelement[i].destination;

				midi_msg_to_synth(&t_msg);
			}
		}


	}


}


void send_CC_to_not_open_trans()
{
	uint8_t cc_thru_flag   = DEFAULT;
	for(int i = 0; i < TRANSCCAMOUNT; i++) // for every cell
	{
		if(main_trans.trelement[i].en != 0
		&& main_trans.trelement[i].open != 0
		&&(incoming_CC_msg_buffer.status & 0xF) == main_trans.trelement[i].parameters[TRANSSRCPos])
		{
			cc_thru_flag = nCCTRANS;
		}
	}
	if(cc_thru_flag == DEFAULT)
	{
		if(msg_type == GOTCHAFTTOUCH)
		{
			midi_chanafttouch_to_synth(&incoming_CC_msg_buffer);
		}
		else
		{
			midi_msg_to_synth(&incoming_CC_msg_buffer);
		}

//		midi_msg_to_synth(&incoming_CC_msg_buffer); //midi cc thru
//		thru_trigger_counter_set();


//		word_to_serial("CCThru:");//debug
//		midi_msg_print(&incoming_CC_msg_buffer); //debug
	}
}



////////////
void incoming_note_processing(int in)
{

	midi_msg_init(&incoming_note_buffer);

	if(in == 1)
	{
		incoming_note_buffer = incoming_note;
		incoming_note.complete_flag = 0;
	}
	else if(in == 2)
	{
		incoming_note_buffer = incoming_note2;
		incoming_note2.complete_flag = 0;
	}

	if(global_mode == SEQMODE)//sequencer mode
	{
		if(sequencer_arr[active_sequencer_index].learn == 1 && delete_process_flag == 0)//sequencer in learn mode and no deleting
		{
			gate_measurement_counter_and_flag_management();
			message_append_to_sequencer(&sequencer_arr[active_sequencer_index]); //tnote
		}
		//else if(sequencer_arr[active_sequencer_index].last_index == 0)//sequencer not in learn but empty
		else if(sequencer_arr[active_sequencer_index].learn == 0)//sequencer not in learn but empty
		{
			//while(midi_access_semaph == 1);
			midi_msg_to_synth(&incoming_note_buffer); //midi thru
			thru_trigger_counter_set();

			//word_to_serial("Thru:");//debug
			//midi_msg_print(&incoming_note_buffer); //debug
		}
	}
	else if(global_mode == POLYMODE)//polyphoniser mode
	{
//		word_to_serial("POLY ");
//		midi_msg_print(&incoming_note_buffer);//debug
		if(main_stack.stelement[main_stack.active_stindex].learn == PLAYMODE)//element not in learn mode
		{
			if((incoming_note_buffer.status & 0xF) == main_cfg.stack[POLYINCHPos]) //main_stack.input_channel)
			{
				push_incoming_note_to_stack(&main_stack, incoming_note_buffer);
				/*// Simple mode Version
				if     ((incoming_note_buffer.status >> 4) == 0x9) //noteOn
					simple_version_push_to_stack(incoming_note_buffer);
				else if((incoming_note_buffer.status >> 4) == 0x8) //noteOff
					simple_version_pop_from_stack(incoming_note_buffer);
				 */
			}
			else
			{
				midi_msg_to_synth(&incoming_note_buffer); //midi thru
				thru_trigger_counter_set();


				//word_to_serial("Thru:");//debug
				//midi_msg_print(&incoming_note_buffer); //debug
			}
			//push_to_waiting_list();
		}
		else if(main_stack.stelement[main_stack.active_stindex].learn == LEARNMODE) //element in learn mode
		{
			midi_msg_to_synth(&incoming_note_buffer);
			main_stack.stelement[main_stack.active_stindex].destination = incoming_note_buffer.status & 0xF; //take channel number
			main_stack.stelement[main_stack.active_stindex].en = 1; //now this element is enabled

			//word_to_serial("DST ");//debug
			//note_stack_element_print(&main_stack.stelement[main_stack.active_stindex]);//debug
		}
	}
	else if(global_mode == MULTMODE)
	{
		//word_to_serial("M ");
		//midi_msg_print(&incoming_note_buffer);//debug
		if(main_mult.mtelement[main_mult.active_mtindex].learn == PLAYMODE)//element not in learn mode
		{
			if((incoming_note_buffer.status & 0xF) == main_cfg.mult[MULTINCHPos]) //from input channel
			{
				//send to ranges destinations
				send_to_mult_dest(&main_mult,incoming_note_buffer);
				//word_to_serial("range processing");
			}
			else
			{
				midi_msg_to_synth(&incoming_note_buffer); //midi thru
				thru_trigger_counter_set();


				//word_to_serial("Thru:");//debug
				//midi_msg_print(&incoming_note_buffer); //debug
			}
		}
		else if(main_mult.mtelement[main_mult.active_mtindex].learn == LEARNMODE) //element in learn mode
		{

			struct midi_msg temp_note;
			midi_msg_init(&temp_note);
			temp_note = incoming_note_buffer;

			if((temp_note.status & 0xF) != main_cfg.mult[MULTINCHPos])  // Not from Input channel
			{
				if ((temp_note.status >> 4) == 0x9) //noteON
				{
					main_mult.mtelement[main_mult.active_mtindex].parameters[border_flag++] = temp_note.data1;
					border_flag %= 2;

					main_mult.mtelement[main_mult.active_mtindex].destination = (temp_note.status & 0xF);
					main_mult.mtelement[main_mult.active_mtindex].en = 1; // element enabled
				}
				// Send note to dest
				uint8_t temp_shift = main_mult.mtelement[main_mult.active_mtindex].parameters[MULTRANGESHIFTPos];
				if(temp_shift == 0)    //collapse to low
				{
					temp_note.data1 = main_mult.mtelement[main_mult.active_mtindex].parameters[MULTSTARTPos];
				}
				else if(temp_shift == 7)// collapse to high
				{
					temp_note.data1 = main_mult.mtelement[main_mult.active_mtindex].parameters[MULTENDPos];
				}
				else if(temp_shift == 15) //inversion
				{
					temp_note.data1 = 127 - temp_note.data1;
				}
				else if(temp_shift < 7)
				{
					while(temp_shift > 0 && temp_note.data1 > 12)
					{
						temp_note.data1 -= 12;
						temp_shift--;
					}
				}
				else if(temp_shift > 8)
				{
					while(temp_shift > 8 && temp_note.data1 < 115)
					{
						temp_note.data1 += 12;
						temp_shift--;
					}
				}
				midi_msg_to_synth(&temp_note);

			}
			else
			{
				send_to_mult_dest(&main_mult,temp_note);
			}
		}
	}
	else if(global_mode == TRANSMODE)
	{
		//Эту функцию нужно бы переместить, но похуй!
//		word_to_serial("Tin ");						//debug
//		midi_msg_print(&incoming_note_buffer);		//debug

//		uint8_t midi_thru_flag = 0; 				// no channel needs to translate this note


		if(main_trans.trelement[main_trans.active_trindex].learn == PLAYMODE)// && (incoming_note_buffer.status >> 4) == 0x9)//element not in learn mode and we got noteOn
		{
			send_to_trans_dest();
		}
		else if(main_trans.trelement[main_trans.active_trindex].learn == LEARNMODE) //element in learn mode
		{

			// Note Channel become trans source channel
			if((incoming_note_buffer.status & 0xF0) == 0x90) // noteOn
			{
				main_trans.trelement[main_trans.active_trindex].parameters[TRANSSRCPos] = incoming_note_buffer.status & 0xF; //take channel number
			}
			else
			{
				midi_msg_to_synth(&incoming_note_buffer);
			}


			midi_msg_init(&temp_trans_note);
			temp_trans_note = incoming_note_buffer;


			if((temp_trans_note.status >> 4 == 0x9))    //need to translate, not midi thru
			{
//				midi_thru_flag = NOTETRANS;				//some channel translates this note
				//translate to CC and send
				temp_trans_note.status  = 0xB0; 														// now it's CC message
				temp_trans_note.status += main_trans.trelement[main_trans.active_trindex].parameters[TRANSDSTPos]; 			// now it's CC message
				temp_trans_note.data2   = temp_trans_note.data1; 											//�� value is note
				temp_trans_note.data1   = main_trans.trelement[main_trans.active_trindex].parameters[TRANSCCPos]; 			//18 - filter1 116 - lfo1 freq 17 - osc2pitch
				midi_msg_to_synth(&temp_trans_note);

			}

			// возможно поменять параметр индекс, чтобы было видно что он поменялся
		}
	}
	else // empty modes
	{
		midi_msg_to_synth(&incoming_note_buffer); //midi thru
		thru_trigger_counter_set();
	}
}

void send_to_trans_dest()
{
	uint8_t midi_thru_flag = DEFAULT; 				// no channel needs to translate this note
	midi_msg_init(&temp_trans_note);
	for(int i = 0; i < TRANSCCAMOUNT; i++) // for every cell
	{
		if(main_trans.trelement[i].en != 0
		&& main_trans.trelement[i].open != 0
		&&(incoming_note_buffer.status & 0xF) == main_trans.trelement[i].parameters[TRANSSRCPos])
		{
			midi_thru_flag = NOTETRANS;
			if(incoming_note_buffer.status >> 4 == 0x9)
			{
				temp_trans_note = incoming_note_buffer;
				temp_trans_note.status  = 0xB0; 														// now it's CC message
				temp_trans_note.status += main_trans.trelement[i].parameters[TRANSDSTPos]; 			// now it's CC message
				temp_trans_note.data2   = temp_trans_note.data1; 											//�� value is note
				temp_trans_note.data1   = main_trans.trelement[i].parameters[TRANSCCPos]; 			//18 - filter1 116 - lfo1 freq 17 - osc2pitch
				//display info
				main_trans.trelement[i].moving_toggles += 2;
				// sending
				midi_msg_to_synth(&temp_trans_note);
			}
		}
	}

	if(midi_thru_flag == DEFAULT) // midi thru
	{
		midi_msg_to_synth(&incoming_note_buffer); //midi thru
		thru_trigger_counter_set();


//		word_to_serial("Thru:");//debug
//		midi_msg_print(&incoming_note_buffer); //debug
	}

}


void mult_borders_swap()
{
	if(main_mult.mtelement[main_mult.active_mtindex].parameters[MULTSTARTPos] >
		main_mult.mtelement[main_mult.active_mtindex].parameters[MULTENDPos])
	{
		uint8_t buf;
		buf = main_mult.mtelement[main_mult.active_mtindex].parameters[MULTENDPos];
		main_mult.mtelement[main_mult.active_mtindex].parameters[MULTENDPos] = main_mult.mtelement[main_mult.active_mtindex].parameters[MULTSTARTPos];
		main_mult.mtelement[main_mult.active_mtindex].parameters[MULTSTARTPos] = buf;
	}

}
void gate_measurement_counter_and_flag_management()
{
	if	   ((incoming_note_buffer.status >> 4) == 0x9)//noteOn
	{
		gate_interval = 0;      //Every time we get noteOn gate_interval is reset
		time_measure_flag = 1;  //start measuring
	}
	else if((incoming_note_buffer.status >> 4) == 0x8)//noteOff
	{
		time_measure_flag = 0;  //stop measuring
	}
}

void reset_all_seq_ind()
{
	for(int i = 0; i < SEQAMOUNT; i++)
	{
		sequencer_arr[i].pop_index = 0;
		sequencer_arr[i].active_notes = 0;
		sequencer_arr[i].trigger_toggles = 0;
	}
}


////////////////////////
////OLD UNUSED FUNCTIONS

void simple_version_push_to_stack(struct midi_msg push_note) //FUNCTION OF OLD VERSION
{
	for(int i = 0; i < STACKHEIGHT; i++)
	{
		if(main_stack.stelement[i].en == 1 && main_stack.stelement[i].open == 1 && main_stack.stelement[i].active == 0) //Element enabled, unmuted, not active
		{
			main_stack.stelement[i].stack_note_left = push_note;
			main_stack.stelement[i].stack_note_left.status &= ~0xF; //reset lower nibble - midi channel
			main_stack.stelement[i].stack_note_left.status += main_stack.stelement[i].destination;
			main_stack.stelement[i].active = 1;

			midi_msg_to_synth(&main_stack.stelement[i].stack_note_left);

			//midi_msg_print(&main_stack.stelement[i].stnote); //debug
//			word_to_serial("+"); //debug
//			note_stack_element_print(&main_stack.stelement[i]); //debug
			break;
		}
	}
	//midi_msg_init(&note); //note init and skip if stack is full
}
void simple_version_pop_from_stack(struct midi_msg pop_note)
{
	for(int i = 0; i < STACKHEIGHT; i++)
	{
		if(pop_note.data1 == main_stack.stelement[i].stack_note_left.data1) // same note
		{
			main_stack.stelement[i].stack_note_left = pop_note;
			main_stack.stelement[i].stack_note_left.status &= ~0xF; //reset lower nibble - midi channel
			main_stack.stelement[i].stack_note_left.status += main_stack.stelement[i].destination;
			main_stack.stelement[i].active = 0;

			midi_msg_to_synth(&main_stack.stelement[i].stack_note_left);

			//midi_msg_print(&main_stack.stelement[i].stnote); //debug
//			word_to_serial("-");//debug
//			note_stack_element_print(&main_stack.stelement[i]); //debug
			break;
		}
	}
	//midi_msg_init(&note); //note init and skip if stack is full
}

