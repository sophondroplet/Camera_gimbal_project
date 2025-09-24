/*
 * music.c
 *
 *  Created on: Feb 4, 2025
 *      Author: Lek
 */
#include "music.h"

uint32_t music_high;
uint32_t prescaler;

void Set_Note(uint32_t note)
{
	TIM6->PSC = note;
	TIM6->EGR = TIM_EGR_UG;
}

void play_music(TIM_HandleTypeDef *handle){

//	TIM6->PSC = prescaler;
	if (TIM6->PSC != prescaler){
		Set_Note(prescaler);
	}
	music_high = (TIM6->CNT & 0b01000000);
	HAL_TIM_Base_Start(handle);

	if (music_high){
		torque = 0.03;
	}else{
		torque = -0.03;
	}
}

