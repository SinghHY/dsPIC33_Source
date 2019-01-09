/**********************************************************************
* Filament Control.
* © Harpreet Singh, 2017
* FileName:        main.c
* Dependencies:    Header (.h) files if applicable, see below* 
* Processor:dsPIC33FJ256GP710A
* Compiler:        MPLAB® CCS v5.074
************************************************************************/
#include <33FJ256GP510.h>
#fuses XT,NOWDT,NOPROTECT
#device ADC = 12 
#use delay(clock = 100Mhz, crystal = 40Mhz)
#use spi(SLAVE, SPI2, BITS = 8, MODE = 1, ENABLE = PIN_G9, stream = SPI_2)

#define Probe_Pin  PIN_E7


int8 i, ProbeId = 0;
int probe, Voltage, Temp;
   
#bank_dma 
    unsigned int8  TxBuffer[4];
#bank_dma 
    unsigned int8  RxBuffer[4]; 

void main()
{    
    
   
   output_float(PIN_G9); // SS as an input
   setup_adc_ports(sAN0 | sAN2 | sAN30, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL);
   
   
   setup_timer2(TMR_INTERNAL | TMR_DIV_BY_64, 500);
   setup_compare(1, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(1,0);
   
   setup_compare(2, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(2,0);
   
   setup_compare(3, COMPARE_PWM | COMPARE_TIMER2);
   set_pwm_duty(3,0);
   
   
   setup_dma(0, DMA_IN_SPI2, DMA_BYTE);   
   dma_start(0, DMA_CONTINOUS ,&RxBuffer[0],3); 
   
   setup_dma(2, DMA_OUT_SPI2, DMA_BYTE); 
   dma_start(2, DMA_CONTINOUS ,&TxBuffer[0],3);

   while(1)
   {

       set_adc_channel(0);
       Voltage = read_adc();
       set_adc_channel(2);
       Temp = read_adc();
       set_adc_channel(30);
       probe = read_adc();
       
       if((probe < 50) & (input(Probe_Pin == 0)))
           ProbeId = 0 ; //no probe//
       if((probe > 200) & (input(Probe_Pin == 1)))
           ProbeId = 3 ; 
       if((probe > 200) & (input(Probe_Pin == 0)))
           ProbeId = 1 ; 
       if((probe < 50) & (input(Probe_Pin == 1)))
           ProbeId = 2 ; 
       
       TxBuffer[4] = ProbeId;
   }
  
}   
