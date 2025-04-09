#ifndef _SYSTEM_H
#define _SYSTEM_H

/*System*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>
#include <stdlib.h>

/*HardWare*/
#include "sys.h"
#include "led.h"
#include "key.h"
#include "rtc.h"
#include "qspi.h"
#include "nand.h"
#include "exti.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "iic.h"
#include "sdram.h"
#include "sdmmc.h"
#include "malloc.h"

/*SoftWare*/
#include "main.h"
#include "store.h"
#include "sys_init.h"
#include "parameter.h"
#include "function.h"
#include "logic_ctrl.h"
#include "modulate.h"
#include "demodulate.h"

/*Arm-Mx*/
#include "arm_math.h"
#include "core_cm7.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "arm_const_structs.h"

#endif /*_SYSTEM_H */




















