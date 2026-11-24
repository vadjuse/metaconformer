/*
 * byte_convertion.c
 *
 *  Created on: 23 ����. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

char msf_char_number[3]; //most significant first

void byte_to_word(uint8_t number)
{
	//Convert byte into two digit chars
	//Resulting string appears in msf_char_number
	if((number & 0xF) < 0xA)
		msf_char_number[1] = (number & 0xF)+'0';
	else
		msf_char_number[1] = (number & 0xF)-0xA + 'A';

	if((number >> 4) < 0xA)
		msf_char_number[0] = (number >> 4) + '0';
	else
		msf_char_number[0] = (number >> 4) - 0xA + 'A';

	    msf_char_number[2] = '\0';
}
void byte_to_midi_device(uint8_t temp_byte)
{
	// non-blocking
	USART1_push_to_queue(temp_byte);
	UART4_push_to_queue(temp_byte);
	//blocking
//	while((USART1->SR & USART_SR_TC)==0);
//	while((UART4->SR  & USART_SR_TC)==0);
//	USART1->DR = temp_byte;
//	UART4->DR  = temp_byte;
}

void byte_to_serial(uint8_t temp_byte)
{
//	while((USART2->SR & USART_SR_TC)==0);
//	USART2->DR = temp_byte;
}

void word_to_serial(char *word)
{
//	for(int i = 0; word[i] != '\0'; i++)
//		byte_to_serial(word[i]);
}

void number_to_serial_human_readable(uint8_t my_num)
{
	//Send converted char string to Serial
	byte_to_word(my_num);
	word_to_serial(msf_char_number);
}
