/*
 * at45dblib.h
 *
 *  Created on: 23 июл. 2021 г.
 *      Author: we
 */
#include "main.h"
#ifndef INC_AT45DBLIB_H_
#define INC_AT45DBLIB_H_



#endif /* INC_AT45DBLIB_H_ */


void at45dblib_getid();
void at45dblib_page_prog_1(uint16_t page_num, uint8_t *data, uint16_t data_size);
void at45dblib_read_status(uint8_t *status_reg);
void at45dblib_wait_ready_bit();
void at45dblib_cont_read(uint16_t page_num, uint8_t *data, uint16_t data_size);
void at45dblib_mass_erase();

