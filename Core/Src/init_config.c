/*
 * init_config.c
 *
 *  Created on: 23 ����. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"
#include "at45db_api.h"


extern struct led 			learn_led;
extern uint8_t 				sweep_flag;
extern uint8_t 				sweep_index;
extern uint8_t 				global_mode_select_counter;


extern uint8_t 				index_show_flag;
extern uint8_t 				global_mode;
extern uint8_t              t_global_mode;
extern uint8_t 				comm_CC_knob;

extern struct midi_msg 		incoming_note;
extern struct midi_msg 		incoming_CC_msg;
extern struct midi_msg 		incoming_CC_msg2;
extern uint8_t 				msg_type;

void  global_mode_setting() 								// Endless loop until user choose operating mode
{
	//sweep_flag = 1;
	while(global_mode == NDEFMODE)
	{
		if(t_global_mode == NDEFMODE)
		{
			sweep_flag = 1;
			if(sweep_index == SWEEPELEMENTS)
				sweep_index = 0;
		}

		message_assambling();
		if(incoming_CC_msg.complete_flag == 1 && (incoming_CC_msg.status >> 4) == 0xB)
		{
			if(comm_CC_knob == 0)
			{
				comm_CC_knob = incoming_CC_msg.data1;

				//debug
				//word_to_serial("knob: ");
				//number_to_serial_human_readable(comm_CC_knob);
				//word_to_serial("\r");
				//debug
			}
			else if(global_mode == NDEFMODE && incoming_CC_msg.data1 == comm_CC_knob)
			{
				t_global_mode = incoming_CC_msg.data2 >> 4;
				global_mode_select_counter = GLOBMODESELECTDELAY;

				//debug
				//word_to_serial("mode: ");
				//number_to_serial_human_readable(t_global_mode);
				//word_to_serial("\r");
				//debug
			}
			//midi_msg_init(&incoming_CC_msg);
			incoming_CC_msg.data1 = 0;
			incoming_CC_msg.data2 = 0;
			incoming_CC_msg.complete_flag = 0;
			incoming_CC_msg.trig = 0;
		}
		message_assambling2();
		if(incoming_CC_msg2.complete_flag == 1 && (incoming_CC_msg2.status >> 4) == 0xB)
		{
			if(comm_CC_knob == 0)
			{
				comm_CC_knob = incoming_CC_msg2.data1;

				//debug
				//word_to_serial("knob: ");
				//number_to_serial_human_readable(comm_CC_knob);
				//word_to_serial("\r");
				//debug
			}
			else if(global_mode == NDEFMODE && incoming_CC_msg2.data1 == comm_CC_knob)
			{
				t_global_mode = incoming_CC_msg2.data2 >> 4;
				global_mode_select_counter = GLOBMODESELECTDELAY;

				//debug
				//word_to_serial("mode: ");
				//number_to_serial_human_readable(t_global_mode);
				//word_to_serial("\r");
				//debug
			}
			//midi_msg_init(&incoming_CC_msg);
			incoming_CC_msg2.data1 = 0;
			incoming_CC_msg2.data2 = 0;
			incoming_CC_msg2.complete_flag = 0;
			incoming_CC_msg2.trig = 0;
		}

		if((GPIOC->IDR & 1 << CFGBTNPIN) == 0)
		{
			  uint8_t glob_data[GLOBCFGBYTES];
			  at45dblib_wait_ready_bit();
			  at45dblib_cont_read(SAVEGLOBPG, glob_data, GLOBCFGBYTES);

			  global_mode  = glob_data[LASTMODEPos];
			  comm_CC_knob = glob_data[LASTCCKNOBPos];
		}
/*
		for(int i = 0; i < SEQAMOUNT; i++)
		{
			if((GPIOC->IDR & 1 << i) == 0)
			{
				global_mode = i;
				break;
			}
		}
*/
	}
	sweep_flag = 1;
	  uint8_t glob_data[GLOBCFGBYTES];
	  at45dblib_wait_ready_bit();
	  at45dblib_cont_read(SAVEGLOBPG, glob_data, GLOBCFGBYTES);

	  glob_data[LASTMODEPos]   = global_mode;
	  glob_data[LASTCCKNOBPos] = comm_CC_knob;

	  at45dblib_wait_ready_bit();
	  at45dblib_page_prog_1(SAVEGLOBPG, glob_data, GLOBCFGBYTES);

//	if(sweep_index == SWEEPELEMENTS)
//		sweep_index = 0;
}

void global_init()
{
//	EXTI_Init();
//	usart1_init_and_start();
//	usart2_init_and_start();
//	usart3_init_and_start();
//	TIM1_Init_and_Start();

	learn_led.gpio  = GPIOB; 							//learn LED
	learn_led.pin   = 2;
	learn_led.state = 0;

//	index_show_set();
//	structures_init();									//initialisation of all structures

}

void usart1_init_and_start()							//To MIDI Device
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; 		  		//USART clock enable
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODER9_Pos;		//alternative function Tx
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODER10_Pos;		//alternative function Rx

	GPIOA->PUPDR |= GPIO_PULLUP << GPIO_PUPDR_PUPD10_Pos;

	GPIOA->AFR[1]|= 0b111 << 4; 						//usart1  pin PA9  AF7
	GPIOA->AFR[1]|= 0b111 << 8; 						//usart1  pin PA10 AF7

	//PCLK2 =
	// 48MHz
	USART1->BRR   = 1536;//2048; 								//baud 31250
	USART1->CR1  |= USART_CR1_RE
	    	  	 |  USART_CR1_TE
				 |  USART_CR1_TCIE
			     |  USART_CR1_UE; 						//usart, tx, rx enable

	NVIC_EnableIRQ(USART1_IRQn);
}

void usart2_init_and_start()							//To COM Port or From MIDI In2
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; 		  		//USART clock enable
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODER2_Pos;		//alternative function Tx
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODER3_Pos;		//alternative function Rx

	GPIOA->PUPDR |= GPIO_PULLUP << GPIO_PUPDR_PUPD3_Pos;

	GPIOA->AFR[0]|= 0b111 << 8; 						//usart2  pin PA2 AF7
	GPIOA->AFR[0]|= 0b111 << 12; 						//usart2  pin PA3 AF7

	//PCLK1 =
	// 24MHz
//	USART2->BRR   = 208;//278; 		   						//baud 115200
	USART2->BRR   = 768;//278; 		   						//baud 31250
	USART2->CR1  |= USART_CR1_RE
	    	  	 |  USART_CR1_TE
				 |  USART_CR1_RXNEIE 					//interrupt then RDR is not empty
			     |  USART_CR1_UE; 						//usart, tx, rx enable
	NVIC_EnableIRQ(USART2_IRQn); 						//enable usart irq
}

void usart3_init_and_start()							//From MIDI device
{
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN; 		  		//USART clock enable
	GPIOC->MODER |= 0b10 << GPIO_MODER_MODER10_Pos;		//alternative function Tx
	GPIOC->MODER |= 0b10 << GPIO_MODER_MODER11_Pos;		//alternative function Rx

	GPIOC->AFR[1]|= 0b111 << GPIO_AFRH_AFSEL10_Pos; 	//usart3  pin PC10 AF7
	GPIOC->AFR[1]|= 0b111 << GPIO_AFRH_AFSEL11_Pos; 	//usart3  pin PC11 AF7

	//PCLK1 =
	//24MHz
	USART3->BRR  = 768;//1024; 								//baud 31250
	USART3->CR1 |= USART_CR1_RE
	    	  	|  USART_CR1_TE
				|  USART_CR1_RXNEIE 					//interrupt then RDR is not empty
			    |  USART_CR1_UE; 						//usart, tx, rx enable

	NVIC_EnableIRQ(USART3_IRQn); 						//enable usart irq
}

void usart4_init_and_start()							//To MIDI device
{
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN; 		  		//UART clock enable
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODER0_Pos;		//alternative function Tx
	//GPIOC->MODER |= 0b10 << GPIO_MODER_MODER11_Pos;		//alternative function Rx

	GPIOA->AFR[0]|= 0b1000 << GPIO_AFRL_AFSEL0_Pos; 	//uart4  pin PA0 AF8
	//GPIOA->AFR[1]|= 0b111 << GPIO_AFRH_AFSEL11_Pos; 	//usart3  pin PC11 AF7

	//PCLK1 =
	//24MHz
	UART4->BRR  = 768;//1024;							//baud 31250
	UART4->CR1 |= USART_CR1_TE
			   |  USART_CR1_TCIE						// TC irq
			   |  USART_CR1_UE; 						//usart, tx, rx enable

	NVIC_EnableIRQ(UART4_IRQn);
}


void GPIO_Init()
{
	RCC->AHB1ENR   |= 1 << RCC_AHB1ENR_GPIOAEN_Pos
    		  	   |  1 << RCC_AHB1ENR_GPIODEN_Pos
			  	   |  1 << RCC_AHB1ENR_GPIOBEN_Pos
				   |  1 << RCC_AHB1ENR_GPIOCEN_Pos; 	//gpio ports enabled

	GPIOA->MODER   |=   1 << GPIO_MODER_MODER4_Pos; 	//clk pin



	GPIOB->MODER 	|= 1 << GPIO_MODER_MODER0_Pos
    				|  1 << GPIO_MODER_MODER1_Pos
					|  1 << GPIO_MODER_MODER2_Pos
					|  1 << GPIO_MODER_MODER3_Pos
					|  1 << GPIO_MODER_MODER4_Pos
					|  1 << GPIO_MODER_MODER5_Pos
					|  1 << GPIO_MODER_MODER6_Pos
					|  1 << GPIO_MODER_MODER7_Pos
					|  1 << GPIO_MODER_MODER8_Pos
					|  1 << GPIO_MODER_MODER9_Pos
					|  1 << GPIO_MODER_MODER12_Pos
					|  1 << GPIO_MODER_MODER14_Pos;
	// Memory configs
	GPIOB->MODER	|= 1 << GPIO_MODER_MODER9_Pos; 		//nWP
	GPIOB->OSPEEDR  |= GPIO_OSPEEDR_OSPEED9;
	GPIOB->MODER	|= 1 << GPIO_MODER_MODER10_Pos; 		//nCS
	GPIOB->OSPEEDR  |= GPIO_OSPEEDR_OSPEED10;


	GPIOB->ODR |=   1<<12; // CTRL B-D = 1 -> DAC
	GPIOB->ODR &= ~(1<<14);// CTRL A-C = 0 -> nMIDI


	/*
	GPIOB->ODR &= ~(1<<12); // CTRL B-D = 0 -> nDAC
	GPIOB->ODR |= (1<<14);  // CTRL A-C = 1 -> MIDI
*/


	sweep_flag = 1;

    GPIOC->PUPDR  |= 0b01 << GPIO_PUPDR_PUPD0_Pos      // buttons from PC0-PC7
    				|0b01 << GPIO_PUPDR_PUPD1_Pos
					|0b01 << GPIO_PUPDR_PUPD2_Pos
					|0b01 << GPIO_PUPDR_PUPD3_Pos
					|0b01 << GPIO_PUPDR_PUPD4_Pos
					|0b01 << GPIO_PUPDR_PUPD5_Pos
					|0b01 << GPIO_PUPDR_PUPD6_Pos
					|0b01 << GPIO_PUPDR_PUPD7_Pos		//////////////////////
					|0b01 << GPIO_PUPDR_PUPD8_Pos		//learn
					|0b01 << GPIO_PUPDR_PUPD9_Pos		//next
					|0b01 << GPIO_PUPDR_PUPD12_Pos		//prev
					|0b01 << GPIO_PUPDR_PUPD13_Pos;		//cfg
}

void EXTI_Init()
{
	//Ornament buttons PC0-7
    //next pc9 prev pc12 learn pc13

    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC; 	//enable GPIOC for source of line 13 IR
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PC; 	//enable GPIOC for source of line 13 IR
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI9_PC;  	//enable GPIOC for source of line 9 IR
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PC;  	//enable GPIOC for source of line 8 IR
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PC;  	//enable GPIOC for source of line 7 IR
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PC;  	//enable GPIOC for source of line 6 IR
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PC;  	//enable GPIOC for source of line 5 IR
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PC;  	//enable GPIOC for source of line 4 IR
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PC;  	//enable GPIOC for source of line 3 IR
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PC;  	//enable GPIOC for source of line 2 IR
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PC;  	//enable GPIOC for source of line 1 IR
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PC;  	//enable GPIOC for source of line 0 IR


	EXTI->IMR |= 0b11001111111111; 					//enabling of interrupts
	EXTI->FTSR|= 0b11000111111111; 					//falling edge PC9- learn is rising
	EXTI->RTSR|= 1 << LEARNBTNPIN;					//learn button work with release

	NVIC_EnableIRQ(EXTI15_10_IRQn); 				//IRQ function enabled
	NVIC_EnableIRQ(EXTI9_5_IRQn); 					//IRQ function enabled
	NVIC_EnableIRQ(EXTI0_IRQn); 					//IRQ function enabled
	NVIC_EnableIRQ(EXTI1_IRQn); 					//IRQ function enabled
	NVIC_EnableIRQ(EXTI2_IRQn); 					//IRQ function enabled
	NVIC_EnableIRQ(EXTI3_IRQn); 					//IRQ function enabled
	NVIC_EnableIRQ(EXTI4_IRQn); 					//IRQ function enabled
}

void TIM1_Init_and_Start()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; 	//tim1 enable
	//PCLK2 APB2 to timers
	// 48MHz
	TIM1->PSC = 479;//639;
	TIM1->ARR = 1000; 						//update event every 10 milliseconds
	TIM1->DIER = TIM_DIER_UIE; 				//update interrupt enable

	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn); 	//TIM1 update interrupt

	TIM1->CR1 |= TIM_CR1_CEN; 				//enable counter
}

void midi_dac_switch_cfg(uint8_t t_i, uint8_t t_d)
{
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
}
