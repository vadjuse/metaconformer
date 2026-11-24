/*
 * functions.h
 */
////global mode constants
#define SEQMODE		   		3
#define POLYMODE	   		1
#define MULTMODE			0
#define TRANSMODE			2
#define NDEFMODE       		9

#define PLAYMODE 			0
#define LEARNMODE			1
#define CFGMODE				2

#define MIDIQUEUEN			2048

////Config array constants
#define MIDICLKPOL			0
#define MIDInCLKPOL			1
#define MIDICLKOFF			2

#define MIDICLKPOLPos		1
#define MIDICLKPSCPos		2			// Prescaler of MIDI CLK
#define MIDITRANSPORTFLTPos 3			// Filtering of start, stop, cont and clk
#define MIDIOUTTHRUPos		0

#define MIDIOUTSW			1
#define DACSYNCSW			0

#define MIDICLKCNTSTOP		0xFF

#define MIDITRANSPORTOPEN	0
#define MIDITRANSPORTCLOSD	1

//Sequencer


//POLY
#define POLYINCHPos			7							//input channel in [7] element



//MULT
#define MULTINCHPos			7							//input channel in [0] element


//TRANS


////structures constants
//SEQUENCER
#define STEPSINSEQ     		16
#define SEQAMOUNT      		8

//POLYSTACK
#define STACKHEIGHT    		8
#define WAITHEIGHT     		128

//MULT
#define MULTTIMBREAMOUNT	8
#define MULTPARAMAMOUNT	    3

#define MULTRANGESHIFTPos	2
#define MULTSTARTPos		0
#define MULTENDPos			1

//TRANS
#define	TRANSPARAMAMOUNT	3
#define TRANSCCAMOUNT		8

#define TRANSSRCPos			1
#define TRANSDSTPos			2
#define TRANSCCPos			0

#define DEFAULT				0
#define	NOTETRANS			1
#define NOTEOFFTRANS		2
#define nCCTRANS			1

////counters constants
#define TOGGLETIME         	30
#define ALARMTIME          	10
#define TRIGGERTIME        	3
#define SWEEPTIME          	3
#define INDEXBLINKREPEATS  	5
#define MUTESREPEATS	   	2
#define DELETESTEPTIME	   	15
#define AUTONOTEOFFTIME		3
#define GLOBMODESELECTDELAY 200
#define PAUSEDEADTIME		100
#define MIDICLKONSTATE		2

#define PSCACTIVECC			0

////display constants
#define SWEEPELEMENTS 		20 						//amount of elements in sweep animation array

////MIDI constants
#define NOTEON   			0x9
#define NOTEOFF  			0x8

#define GOTNOTE 			0
#define GOTCC				1
#define GOTOTHER			2
#define GOTPITCHBEND		3
#define GOTAFTERTOUCH		4
#define GOTCHAFTTOUCH		5

//// hardware constants
#define LEARNLEDPIN 2
#define LEARNBTNPIN 9//8
#define PREVBTNPIN  12
#define NEXTBTNPIN  8//9
#define CFGBTNPIN	13

#define LEARNLEDPORT 			GPIOB

#define CLKLEDPORT				GPIOA
#define CLKLEDPIN				4


////// MEMORY
#define SAVEGLOBPG				0x300
#define GLOBCFGBYTES			6
#define LASTMODEPos				4
#define LASTCCKNOBPos			5
//
// bytes 0:2  - firmvare version
// byte  3    - need write factory presets flag, 1 -> load FACTXXPG -> SAVEXXPG
// byte  4 	  - last active working MODE
// byte  5    - last CC knob
// Memory chip has 100k R/W cycles per page
//
// Fact - factory page
#define FACTCFGPG				0x301
#define FACTMULTPG				0x302
#define FACTPOLYPG				0x303
#define FACTTRANSPG				0x304
#define FACTSEQPG				0x305

#define SAVECFGPG				0x30F
#define SAVEMULTPG				0x310
#define SAVEPOLYPG				0x320
#define SAVETRANSPG				0x330
#define SAVESEQPG0				0x340
#define SAVESEQPG1				0x341
#define SAVESEQPG2				0x342
#define SAVEXXXPG				0x350




struct led{
	uint8_t pin;
	uint8_t state;
	GPIO_TypeDef *gpio;
};

struct midi_msg{
	uint8_t status; 								//status and channel
	uint8_t data1;
	uint8_t data2;
	uint8_t trig;		 							//trig flag
	uint8_t complete_flag;

};

struct  midi_container{
	struct midi_msg note[STEPSINSEQ];
	uint8_t last_index; 							//last message in container
	uint8_t temp_index; 							//����� �������?
	uint8_t pop_index;  							//pop pointer position
	uint8_t learn;      							//learn mode on/off
	uint8_t active_notes; 							//how much noteOn messages is sended
	uint8_t trigger_toggles; 						//how much triggers is sended

};

struct note_stack_element{
	struct midi_msg stack_note_left;
	struct midi_msg stack_note_right;

	uint8_t destination; 							//midi channel
	uint8_t open;
	uint8_t active;
	uint8_t en;
	uint8_t learn;
	uint8_t moving_toggles;

};

struct note_stack{
	struct note_stack_element stelement[WAITHEIGHT];
	uint8_t max_index; 								//amount of free slots in stack
	uint8_t active_stindex; 						//index of active element
	uint8_t input_channel;
};

struct poly_stack{
	struct midi_msg poly_note_array[WAITHEIGHT];
	uint8_t active_plindex;

	uint8_t destination; 							//midi channel
	uint8_t open;
	uint8_t active;
	uint8_t en;
	uint8_t learn;
	uint8_t moving_toggles;
};


struct poly_stack_array{
	struct poly_stack plelement[STACKHEIGHT];
	uint8_t input_channel;
	uint8_t last_put_index;
};

struct note_range{
	uint8_t parameters[MULTPARAMAMOUNT];			//dest, start, end
	uint8_t active_mtparamindex;

	uint8_t destination;
	uint8_t open;
	uint8_t active;
	uint8_t en;
	uint8_t learn;
	uint8_t moving_toggles;
};

struct note_range_array{
	struct note_range mtelement[MULTTIMBREAMOUNT];
	uint8_t max_index;
	uint8_t active_mtindex;
	uint8_t input_channel;
};

struct note_trans{
	uint8_t parameters[TRANSPARAMAMOUNT];
	uint8_t active_trparamindex;

	uint8_t destination;
	uint8_t source;
	uint8_t open;
	uint8_t active;
	uint8_t en;
	uint8_t learn;
	uint8_t moving_toggles;
};

struct note_trans_array{
	struct note_trans trelement[TRANSCCAMOUNT];
	uint8_t max_index;
	uint8_t active_trindex;
//	uint8_t input_channel;
};

struct config_arrays{
	uint8_t seq[SEQAMOUNT];
	uint8_t stack[STACKHEIGHT];
	uint8_t mult[MULTTIMBREAMOUNT];
	uint8_t trans[TRANSCCAMOUNT];
	//����� �������� � ����� ��������, �� �� ��� ����� ����� ��������� �� ������
	uint8_t max_index_seq;
	uint8_t max_index_poly;
	uint8_t max_index_mult;
	uint8_t max_index_trans;
	uint8_t active_index;
};
/////////HARDWARE Init and Config
void global_init(); 								//Initialisation of all peripherals
void global_mode_setting();							//Setting global mode on Start
void structures_init(); 							//Initialisation of all structures
void usart1_init_and_start();
void usart2_init_and_start();
void usart3_init_and_start();
void usart4_init_and_start();							//To MIDI device

void USART1_IRQHandler();
void USART1_push_to_queue(uint8_t t_byte);

void UART4_IRQHandler();
void UART4_push_to_queue(uint8_t t_byte);

void GPIO_Init();
void EXTI_Init();
void TIM1_Init_and_Start();
void TIM2_Init_and_Start();

void EXTI0_IRQHandler();							//IRQs
void EXTI1_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void EXTI4_IRQHandler();
void EXTI9_5_IRQHandler();
void line_procedure(uint8_t line);
void EXTI15_10_IRQHandler();

void USART2_IRQHandler();		 	 //This USART receives notebook keyboard messages and transmit to notebook COM Port
void USART3_IRQHandler();		 	 //This USART receives midi keyboard messages
void TIM1_UP_TIM10_IRQHandler(); 	 //TIM1 update
void TIM2_IRQHandler();          	 //TIM2 update event

//////////BYTES to CHARS and Vice Versa
void byte_to_word(uint8_t number);						//Converts byte 0xXY to string = "XY"+'\0'
void byte_to_serial(uint8_t my_byte);					//Sends byte to USART2
void byte_to_midi_device(uint8_t my_byte);      		//Sends byte to USART3
void word_to_serial(char *word);	            		//Sends array of bytes to USART3
void number_to_serial_human_readable(uint8_t my_num);	//Converts number to word and send it to USART3

void led_state_toggle_and_show(struct led *led_pointer); //Change struct led state and toggle corr. ODR
void led_display_init_sweep(); 							 //Show that all connected leds work and in right order
void led_display_config_and_show(uint8_t data);          //show binary number on led_display
void display_one_led_toggle(uint8_t led_number);         //toggle just one led


///////SYSTEM State and Feedback
void 	display_show(); 								//show all visual feedback
uint8_t sequencer_work_display_data();
uint8_t poly_work_display_data();
uint8_t mult_work_display_data();
uint8_t trans_work_display_data();

uint8_t learning_sequencer_last_index(struct midi_container learning_sequencer,
									  struct midi_container learning_sequencer_buffer);
uint8_t learning_stelement_destination(struct note_stack_element learning_stelement,
									   struct note_stack_element learning_stelement_buffer);
uint8_t learning_mtelement_destination(struct note_range learning_mtelement,
									   struct note_range learning_mtelement_buffer);


///////COMMON
void index_blink_control();
void cfg_index_blink_control();
void global_mode_select_counter_control();
void sweep_animation_phase_control();

void midi_dac_switch_cfg(uint8_t t_i, uint8_t t_d);

void thru_trigger_counter_control();
void thru_trigger_counter_set();
////SEQ
void seq_alarm_counter_control();
void seq_delete_counter_control();
void seq_trigger_counter_control();
void empty_trig_queue();
void seq_trigger_noteOff_counter_control();
void pause_dead_time_counter_control();
////POLY
void poly_delete_step_counter_control();
void poly_trigger_counter_control();
////MULT
void mult_delete_step_counter_control();
void mult_trigger_counter_control();
////TRANS
void trans_delete_step_counter_control();
void trans_trigger_counter_control();


void counters_and_flags_control();						//control all counters
void index_show_set();  								//init of index showing after next seq or prev seq or end of learn
void show_mutes_set();
void delete_procedure_reset();
void sweep_show_reset();
void trigger_noteOff_counter_set();
void midi_clk_state_control();

///////STRUCTURES Init and Print
void note_stack_element_init(struct note_stack_element *stelement);
void note_stack_element_print(struct note_stack_element *stelement);

void note_stack_init(struct note_stack *stack);
void note_stack_print(struct note_stack *stack);

void poly_stack_init(struct poly_stack *plelement);
void poly_stack_print(struct poly_stack *plelement);

void poly_stack_array_init(struct poly_stack_array *plarray);
void poly_stack_array_print(struct poly_stack_array *plarray);


void note_range_array_init(struct note_range_array *range_array);
void note_range_init(struct note_range *note_range);

void note_range_print(struct note_range *note_range);
void note_range_array_print(struct note_range_array *range_array);

void note_trans_array_init(struct note_trans_array *trans_array);
void note_trans_init(struct note_trans *note_trans);

void note_trans_print(struct note_trans *note_trans);
void note_trans_array_print(struct note_trans_array *trans_array);

void cfg_init(struct config_arrays *config);
void cfg_store_to_at45db();

void midi_msg_print(struct midi_msg *note); 					//Print midi_message object to COM
void midi_msg_to_synth(struct midi_msg *note); 					//Send midi_message to synth
void midi_chanafttouch_to_synth(struct midi_msg *note);

void midi_msg_init(struct midi_msg *note);  					//Initialisation of midi_message object
void sequencer_array_init(struct midi_container *sequencer);
void forced_note_off(struct midi_container *active_sequencer); 	//send noteOff message for every noteOn message in active sequencer
void send_noteOff_for_noteOn(struct midi_msg tnote);			//change status byte from 0x9n to 0x8n and send

void midi_container_init(struct midi_container *sequencer); 	//Initialisation of midi_container object
void midi_container_print(struct midi_container *sequencer,
											  uint8_t num); 	//Print midi_container object


///////MIDI Message functions
void byte_receive_from_usart3();									//Receiving byte and push it to byte waitlist
void byte_receive_from_usart2();									//Receiving byte and push it to byte waitlist

void message_assambling(); 										//analysis of byte status to status, dataX to dataX
void find_place_in_midi_msg(uint8_t t_byte);
void message_assambling2(); 										//analysis of byte status to status, dataX to dataX
void find_place_in_midi_msg2(uint8_t t_byte);


void incoming_note_processing(int in); 								//Action depends on MODE

void incoming_CC_processing(int in);
void comm_knob_CC_processing();
void normal_CC_processing();

void send_CC_to_active_ranges();
void send_CC_to_active_destinations();


///////SEQUENCER Functions
void sequencer_learn_mode_toggle();
void sequencer_end_of_learn  (struct midi_container *active_sequencer);
void sequncer_start_of_learn(struct midi_container *active_sequencer);

void next_sequencer();
void real_next_sequencer(struct midi_container *active_sequencer);
void prev_sequencer();
void real_prev_sequencer(struct midi_container *active_sequencer);

void pop_from_sequencer(int index); 							//Actions assigned to buttons
void real_pop_from_sequencer(struct midi_container *pop_sequencer, uint8_t index);

void message_append_to_sequencer(struct midi_container *sequencer); 		//put message to last_index of learning sequencer
void pause_append_to_sequencer(struct midi_container *learning_sequencer);
void gate_measurement_counter_and_flag_management(); 						//processing of gate_interval and gate_measuring_flag

void reset_all_seq_ind();

///////STACK Functions
void note_stack_learn_mode_toggle();
void stack_end_of_learn(struct note_stack_element *active_stelement);
void stack_start_of_learn(struct note_stack_element *active_stelement);

void prev_stelement();
void real_prev_stelement(struct note_stack_element *active_stelement, uint8_t *stindex);
void next_stelement();
void real_next_stelement(struct note_stack_element *active_stelement, uint8_t *stindex);

void stelement_mute_toggle(int index);
void real_stelement_mute_toggle(struct note_stack *m_stack,struct note_stack_element *pressed_stelement ,uint8_t index);

void push_incoming_note_to_stack(struct note_stack *stack, struct midi_msg push_note);
void push_note_to_stack_element (struct note_stack_element *stack_element, struct midi_msg);
void move_noteOn_from_waiting_to_sounding_stack_element(struct note_stack *stack, struct note_stack_element *dest_stelement);
void delete_notes_from_stack_element(struct note_stack_element *stack_element);
int  find_pair_index(struct note_stack *stack, struct midi_msg push_note);
int  find_free_slot_index(struct note_stack *stack, struct midi_msg push_note);//, int start_index);
int  find_waiting_noteOn_index(struct note_stack *stack, int start_index);


////////MULT Functions
void prev_mtelement();
void next_mtelement();
void real_prev_mtelement(struct note_range_array *range_array);
void real_next_mtelement(struct note_range_array *range_array);
void prev_mtparam();
void next_mtparam();
void real_next_mtparam(struct note_range *range);
void real_prev_mtparam(struct note_range *range);
void start_range_note_trigger();
void end_range_note_trigger();
void real_start_range_note_trigger(struct note_range *range);
void real_end_range_note_trigger(struct note_range *range);

void note_range_learn_mode_toggle();
void mult_end_of_learn(struct note_range *active_mtelement);
void mult_borders_swap();
void mult_start_of_learn(struct note_range *active_mtelement);

void mtelement_mute_toggle(int index);
void real_mtelement_mute_toggle(struct note_range_array *range, struct note_range *pressed_mtelement ,uint8_t index);

void 	send_to_mult_dest(struct note_range_array *range_array, struct midi_msg sending_note);
uint8_t mult_note_transpose(struct note_range *range, uint8_t note_number);


//////////TRANS Functions
void prev_trelement();
void next_trelement();
void real_prev_trelement(struct note_trans_array *trans_array);
void real_next_trelement(struct note_trans_array *trans_array);
void prev_trparam();
void next_trparam();
void real_next_trparam(struct note_trans *translator);
void real_prev_trparam(struct note_trans *translator);

void note_trans_learn_mode_toggle();
void trans_end_of_learn(struct note_trans *active_trelement);
void trans_start_of_learn(struct note_trans *active_trelement);

void trelement_mute_toggle(int index);
void real_trelement_mute_toggle(struct note_trans_array *trans_array, struct note_trans *pressed_trelement ,uint8_t index);

void send_to_trans_dest();
void send_CC_to_not_open_trans();

/////////////////
////OLD FUNCTIONS
void simple_version_push_to_stack(struct midi_msg push_note);
void simple_version_pop_from_stack(struct midi_msg pop_note);
