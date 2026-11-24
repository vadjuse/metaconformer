/*
 * at45dblib.c
 *
 *  Created on: 23 июл. 2021 г.
 *      Author: we
 */
#include "main.h"
#include "functions.h"
//#include "usart2.h"
extern SPI_HandleTypeDef hspi1;

void nCS_activate()
{
	GPIOB->ODR &= ~(1 << 10); 		//nCS   initial state
}
void nCS_deactivate()
{
	GPIOB->ODR |= 1 << 10; 			//nCS   initial state
}

void at45dblib_getid(uint8_t *rx_arr)
{
	uint8_t id_command = 0x9F;
	nCS_activate();
	HAL_SPI_Transmit(&hspi1, &id_command, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive (&hspi1, rx_arr, 5, HAL_MAX_DELAY);
	nCS_deactivate();
}

void at45dblib_page_prog_1(uint16_t page_num, uint8_t *data, uint16_t data_size)
{
	HAL_StatusTypeDef res1, res2;
	//uint16_t page_num = 5;     // 0x0 - 0xFFF
	uint16_t start_byte = 0;  // 0x0 - 0x107
	uint8_t cmd_seq[4];
	cmd_seq[0] = 0x82;
	//
	// 0b00000000 00000000 00000000
	// 0bxxxPPPPP PPPPPPPB BBBBBBBB
	//
	cmd_seq[1]  = page_num >> 7;
	cmd_seq[2]  = (page_num << 1) & 0xFE;
	cmd_seq[2] += start_byte >> 8;
	cmd_seq[3]  = start_byte & 0xFF;


	nCS_activate();
	res1 = HAL_SPI_Transmit(&hspi1, cmd_seq, 4, HAL_MAX_DELAY);
	res2 = HAL_SPI_Transmit(&hspi1, data, data_size, HAL_MAX_DELAY);
	nCS_deactivate();

	if((res1 != HAL_OK)||(res2 != HAL_OK))
	{
		char* msg = "SPI WR ERROR\r";
		word_to_serial(msg);
		//line_tx((uint8_t*)msg);
	}
}
void at45dblib_read_status(uint8_t *status_reg)
{
	HAL_StatusTypeDef res_tx, res_rx;
	uint8_t cmd = 0xD7;

	nCS_activate();
	res_tx = HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	res_rx = HAL_SPI_Receive(&hspi1, status_reg, 2, HAL_MAX_DELAY);
	nCS_deactivate();

	if((res_tx != HAL_OK)||(res_rx != HAL_OK))
	{
		char* msg = "SPI STATUS ERROR\r";
		word_to_serial(msg);
		//line_tx((uint8_t*)msg);
	}
}

void at45dblib_cont_read(uint16_t page_num, uint8_t *data, uint16_t data_size)
{
	HAL_StatusTypeDef res1, res2;
	//uint16_t page_num = 5;     // 0x0 - 0xFFF
	uint16_t start_byte = 0;  // 0x0 - 0x107
	uint8_t cmd_seq[4];
	//opcode
	cmd_seq[0] = 0x3;
	//
	// 0b00000000 00000000 00000000
	// 0bxxxPPPPP PPPPPPPB BBBBBBBB
	//
	cmd_seq[1]  = page_num >> 7;
	cmd_seq[2]  = (page_num << 1) & 0xFE;
	cmd_seq[2] += start_byte >> 8;
	cmd_seq[3]  = start_byte & 0xFF;


	nCS_activate();
	res1 = HAL_SPI_Transmit(&hspi1, cmd_seq, 4, HAL_MAX_DELAY);
	res2 = HAL_SPI_Receive(&hspi1, data, data_size, HAL_MAX_DELAY);
	nCS_deactivate();

	if((res1 != HAL_OK)||(res2 != HAL_OK))
	{
		char* msg = "SPI RD ERROR\r";
		word_to_serial(msg);
		//line_tx((uint8_t*)msg);
	}
}

void at45dblib_wait_ready_bit()
{
	uint8_t st_reg[2];
	while(1)
	{
		at45dblib_read_status(st_reg);
		if((st_reg[0] & 1 << 7)!= 0) //ready
			break;
	}
}


void at45dblib_mass_erase()
{
	HAL_StatusTypeDef res1;
	uint8_t cmd[4];
	cmd[0] = 0xC7;
	cmd[1] = 0x94;
	cmd[2] = 0x80;
	cmd[3] = 0x9A;

	nCS_activate();
	res1 = HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
	nCS_deactivate();

	if(res1 != HAL_OK)
	{
		char* msg = "SPI MASSER ERROR\r";
		word_to_serial(msg);
		//line_tx((uint8_t*)msg);
	}
}
