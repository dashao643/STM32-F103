#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/

// This function handles Non maskable interrupt.
void NMI_Handler(void)
{
    while (1)
    {
    }
}

// This function handles Hard fault interrupt.
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

// This function handles Memory management fault.
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

// This function handles Prefetch fault, memory access fault.
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

// This function handles Undefined instruction or illegal state.
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

// This function handles System service call via SWI instruction.
void SVC_Handler(void)
{
}

// This function handles Debug monitor.
void DebugMon_Handler(void)
{
}

// This function handles Pendable request for system service.
void PendSV_Handler(void)
{
}

// This function handles System tick timer.
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/
