/*
 * music.h
 *
 *  Created on: Sep 14, 2024
 *      Author: david
 */
#include "stm32g4xx_hal.h"

#ifndef INC_MUSIC_H_
#define INC_MUSIC_H_

#define	NOTE_C0		80265
#define	NOTE_CS0	75762
#define	NOTE_D0		71510
#define	NOTE_DS0	67498
#define	NOTE_E0		63707
#define	NOTE_F0		60132
#define	NOTE_FS0	56757
#define	NOTE_G0		53571
#define	NOTE_GS0	50564
#define	NOTE_A0		47727
#define	NOTE_AS0	45049
#define	NOTE_B0		42520

#define	NOTE_C1		40134
#define	NOTE_CS1	37881
#define	NOTE_D1		35755
#define	NOTE_DS1	33748
#define	NOTE_E1		31854
#define	NOTE_F1		30066
#define	NOTE_FS1	28379
#define	NOTE_G1		26786
#define	NOTE_GS1	25283
#define	NOTE_A1		23864
#define	NOTE_AS1	22524
#define	NOTE_B1		21260

#define	NOTE_C2		20067
#define	NOTE_CS2	18940
#define	NOTE_D2		17878
#define	NOTE_DS2	16874
#define	NOTE_E2		15927
#define	NOTE_F2		15033
#define	NOTE_FS2	14189
#define	NOTE_G2		13393
#define	NOTE_GS2	12641
#define	NOTE_A2		11932
#define	NOTE_AS2	11262
#define	NOTE_B2		10630

#define	NOTE_C3		10033
#define	NOTE_CS3	9470
#define	NOTE_D3		8939
#define	NOTE_DS3	8437
#define	NOTE_E3		7964
#define	NOTE_F3		7517
#define	NOTE_FS3	7095
#define	NOTE_G3		6696
#define	NOTE_GS3	6321
#define	NOTE_A3		5966
#define	NOTE_AS3	5631
#define	NOTE_B3		5315

#define	NOTE_C4		5017
#define	NOTE_CS4	4735
#define	NOTE_D4		4469
#define	NOTE_DS4	4219
#define	NOTE_E4		3982
#define	NOTE_F4		3758
#define	NOTE_FS4	3547
#define	NOTE_G4		3348
#define	NOTE_GS4	3160
#define	NOTE_A4		2983
#define	NOTE_AS4	2816
#define	NOTE_B4		2658

#define	NOTE_C5		2508
#define	NOTE_CS5	2368
#define	NOTE_D5		2235
#define	NOTE_DS5	2109
#define	NOTE_E5		1991
#define	NOTE_F5		1879
#define	NOTE_FS5	1774
#define	NOTE_G5		1674
#define	NOTE_GS5	1580
#define	NOTE_A5		1491
#define	NOTE_AS5	1408
#define	NOTE_B5		1329

#define	NOTE_C6		1254
#define	NOTE_CS6	1184
#define	NOTE_D6		1117
#define	NOTE_DS6	1055
#define	NOTE_E6		995
#define	NOTE_F6		940
#define	NOTE_FS6	887
#define	NOTE_G6		837
#define	NOTE_GS6	790
#define	NOTE_A6		746
#define	NOTE_AS6	704
#define	NOTE_B6		664

#define	NOTE_C7		627
#define	NOTE_CS7	592
#define	NOTE_D7		559
#define	NOTE_DS7	527
#define	NOTE_E7		498
#define	NOTE_F7		470
#define	NOTE_FS7	443
#define	NOTE_G7		419
#define	NOTE_GS7	395
#define	NOTE_A7		373
#define	NOTE_AS7	352
#define	NOTE_B7		332

#define	NOTE_C8		314
#define	NOTE_CS8	296
#define	NOTE_D8		279
#define	NOTE_DS8	264
#define	NOTE_E8		249
#define	NOTE_F8		235
#define	NOTE_FS8	222
#define	NOTE_G8		209
#define	NOTE_GS8	198
#define	NOTE_A8		186
#define	NOTE_AS8	176
#define	NOTE_B8		166

#define	NOTE_C9		157
#define	NOTE_CS9	148
#define	NOTE_D9		140
#define	NOTE_DS9	132
#define	NOTE_E9		124
#define	NOTE_F9		117
#define	NOTE_FS9	111
#define	NOTE_G9		105
#define	NOTE_GS9	99

extern uint32_t music_high;
extern uint32_t prescaler;
extern volatile float torque;

void Set_Note(uint32_t note);
void play_music(TIM_HandleTypeDef *handle);

#endif /* INC_MUSIC_H_ */
