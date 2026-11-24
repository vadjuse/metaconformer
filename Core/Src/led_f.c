/*
 * led_f.c
 *
 *  Created on: 23 ����. 2021 �.
 *      Author: Vadim
 */
#include "main.h"
#include "functions.h"

#define LEDNUMBER 8
#define LOCALSWEEPTIME 400000

//uint8_t led_display_pins[] = {0, 1, 2, 8, 9, 5, 6, 7};

uint8_t led_display_pins[] = {0,8, 7, 6 , 5, 4 ,3, 1};


void led_state_toggle_and_show(struct led *led_pointer)
{
	if(led_pointer->state == 0){
		led_pointer->state = 1;
	}
	else led_pointer->state = 0;

	led_pointer->gpio->ODR ^= 1 << led_pointer->pin;
}


void led_display_init_sweep() //���� ����������
{
	uint8_t my_data = 0 ;
	for(int j = 0; j < LEDNUMBER ; j++ )
	{
		//GPIOB->ODR |= 1 << led_display_pins[j];
		my_data = 1 << j;
		led_display_config_and_show(my_data);
		for(int i = 0; i < LOCALSWEEPTIME; i++);
		led_display_config_and_show(0);
		//GPIOB->ODR &= ~(1 << led_display_pins[j]);
	}
	led_display_config_and_show(0b11111111);
	for(int i = 0; i < 3*LOCALSWEEPTIME; i++);
	//led_display_config_and_show(1<<0);
}

void led_display_config_and_show(uint8_t data)
{
	GPIOB->ODR &= ~0b111111011;
	for(int j = 0; j < LEDNUMBER ; j++ )
	{
		if(data & (1 << j))
			GPIOB->ODR |= 1 << led_display_pins[j];
	}
}

void display_one_led_toggle(uint8_t led_number)
{
	GPIOB->ODR ^= 1 << led_display_pins[led_number];
}
