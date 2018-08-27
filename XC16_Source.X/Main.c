/**********************************************************************
* Temperature Control 
* © PerkinElmer Health Sciences Canada, Inc., 2017
* This program is for Heater control U26 on old Board. 
* FileName:        Main.c
* Processor:       dsPIC33FJ256GP510A
* Compiler:        XC16 v1.35 or Higher
* Version:          1
* IC                U22, U26 on Source Board
************************************************************************/

#define P_Gain 2 //13// 
#define D_Gain 400 //621//
#define I_Gain 15 //13// 


#include "p33Fxxxx.h"
#include <spi.h>
#include <outcompare.h>
#include <timer.h>

_FGS(GSS_OFF & GWRP_OFF); //code protect off,write protect disabled
_FOSCSEL(FNOSC_PRIPLL & IESO_OFF);
_FOSC(FCKSM_CSECME & OSCIOFNC_OFF & POSCMD_HS);
_FWDT(FWDTEN_OFF);//watch dog disabled

// configuration settings//
int SPIFlag =0 ;
int rxdData1;
unsigned short CommandCounter1 = 0;
int txdData1;
int TempAverageOld = 0;
int AverageCounter = 0;
unsigned int ControlledBand = 15; 
unsigned int IntegralCounter = 0;
unsigned int IntegralCounterMax = 34000;// 27021;//60021
unsigned int IntegralCounterDec = 320000;//1921; 
unsigned int Integral=0 ;
int BoostCounter =0;
int IntegralBoost = 1;
int LoopCounter = 0;
unsigned int RampTime =0;
unsigned short MyResult = 0;
// Functions //
unsigned short Calculate ( unsigned short TempSetpoint, unsigned short Temperature);


//*************************************************************************************//
//SPI2 Interrupt //
void __attribute__((__interrupt__,no_auto_psv)) _SPI2Interrupt(void) 
{    
    IFS2bits.SPI2IF = 0;
    SPI2STATbits.SPIROV = 0;  // Clear SPI1 receive overflow flag if set //

    PORTGbits.RG15 = 1 ;
    rxdData1 = ReadSPI2();

    SPIFlag =1;
    CommandCounter1 = CommandCounter1 +1 ;
    
    if (!SPI2STATbits.SPITBF)
        WriteSPI2(txdData1);//if txd buffer is rdy send data//
}


//---------------------------- Main loop-------------------------------------------//
int main(void)
{

int Instruction1; // 1,2,3
int DisplayCounter =0;
long int TempDisplay =1;
long int TempAverage = 1;
long int TempAverageCopy = 1;



unsigned short ResultHV; 
unsigned short ResultTemperature;
unsigned short ResultTemperatureCopy;
unsigned short Temperature;
unsigned short TempSetpoint = 0;
unsigned short HVSetpoint1 = 0;
unsigned short HVSetpoint2 = 0;
unsigned char TempSetpointLow=0;
unsigned char TempSetpointHi=0;
unsigned char TemperatureLowDisplay=0;
unsigned char TemperatureHiDisplay=0;
unsigned char VoltageMonitorLow1=0;
unsigned char HVSetpointLow1=0;
unsigned char HVSetpointHi1=0;
unsigned char APCIRealTime=0;
unsigned char ESIRealTime=0;
unsigned char ProbeId=0x0, version = 0x1;



// init the SPI 1 and SPI2

    SPI2CON1 = 0x8080;  //enable slave, mode8, cke=1, ckp = 0, smp = 0
    SPI2STAT = 0x8000;	//0x8080 enables the spi

 
    TRISG = 0x00;
    //init PWM for HV1 and HV2 , fc 90KHZ //

    CloseOC1();
    ConfigIntOC1(OC_INT_OFF );
    OpenOC1(OC_IDLE_CON & OC_TIMER2_SRC & OC_CONTINUE_PULSE, 316, 318);

    CloseOC2();
    ConfigIntOC2(OC_INT_OFF );
    OpenOC2(OC_IDLE_CON & OC_TIMER2_SRC & OC_CONTINUE_PULSE, 316, 318);


        //init timer2, 0x175 for 90khz//
    ConfigIntTimer2(T2_INT_OFF);
    WriteTimer2(0);
    OpenTimer2(T2_ON & T2_GATE_OFF & T2_PS_1_1 ,  318);
    T2CON = 0x8000;

        //init PWM for Temp1 and Temp2 , fc 60 HZ //
    CloseOC3();
    ConfigIntOC3(OC_INT_OFF );
    OpenOC3(OC_IDLE_CON & OC_TIMER3_SRC & OC_CONTINUE_PULSE, 0xfffd, 0xffff);
    SetPulseOC3(0x0, 0xfffd);
 
        //init timer3, 0xffff for 60Hz//
    ConfigIntTimer3(T3_INT_OFF);
    WriteTimer3(0);
    OpenTimer3(T3_ON & T3_GATE_OFF & T3_PS_1_1 ,  0xfffe);
    T3CON = 0x8010;

        //init ADC channels for AN0 and AN2 //
	AD1CON1 = 0x00e0;
	AD1CON2 = 0x0000;
 	AD1CON3 = 0x1f02;
    AD1PCFGL = 0xfffa; //AN0 and AN2 ADC enabled 
    AD1PCFGH = 0xbfff; //needed for probe ID reading and an30 enabled//
    AD1CSSH = 0x0000;     
	AD1CSSL = 0x0000;  //ano and an2 scan 			
	AD1CON1bits.ADON =1; // ADC1 on

/* Configure SPI2 interrupt */
    ConfigIntSPI2(SPI_INT_EN &  SPI_INT_PRI_6);
    

// start of main loop !!!! //
while(1)
 {
    if (( MyResult < 50) & (PORTEbits.RE7 == 0))
        {
            ESIRealTime = 0 ;
            APCIRealTime = 0;
            ProbeId = 0 ; //no probe//
        }   
    if (( MyResult > 200) & (PORTEbits.RE7 == 1))
        {
            ESIRealTime = 1 ;
            APCIRealTime = 0;
            ProbeId = 3 ; //APPI probe//
        }   
    if (( MyResult > 200) & (PORTEbits.RE7 == 0))
        {
            ESIRealTime = 1 ;
            APCIRealTime = 0;
            ProbeId = 1 ; //ESI probe//
        }   
    if (( MyResult < 50) & ((PORTEbits.RE7) == 1))
        {
            ESIRealTime = 0 ;
            APCIRealTime = 1;
            ProbeId = 2 ; //APCI probe//
        }
   
    if (( MyResult > 100)&( MyResult < 200))
        {
            ESIRealTime = 1 ;
            APCIRealTime = 0;
            ProbeId = 4 ; //Micro spray//
        }   

		 //Check the APCI and ESI logic for protection //

    if  ( APCIRealTime == 1) //(ESIRealTime == 0)// 
        {
            HVSetpointHi1 = 0;    // reset HV //
            HVSetpointLow1 = 0;   // reset HV // 
        }		
    if  (ESIRealTime == 0) 
        {
            HVSetpointHi1 = 0;    // reset HV //
            HVSetpointLow1 = 0;   // reset HV // 
        }		
         //time to check spi flag//
    if(SPIFlag == 1)
        {   
            SPIFlag = 0;
        
            switch(CommandCounter1) 
                {
                    case 4: 
                            CommandCounter1 = 0;
                            txdData1 = version;
                            if(Instruction1 == 1)
                                TempSetpointLow = rxdData1;
                            else if((Instruction1 == 2) ^ (Instruction1 == 12))
                                     HVSetpointLow1 = rxdData1;
                            break;
                    case 1:      
                             Instruction1 = rxdData1;
                             if ((Instruction1 == 1) ^ (Instruction1 == 11))
                                  txdData1=0;
                             else if ((Instruction1 == 2) ^ (Instruction1 == 12))
                                      txdData1=VoltageMonitorLow1;
                             else if (Instruction1 == 3)
                                      txdData1=ProbeId;
                            break;
                    case 2:   
                             if ((Instruction1 == 1)^(Instruction1 == 11))	
                                  txdData1=0;
                             else if ((Instruction1 == 2)^(Instruction1 == 12))
                                   txdData1=0;     
                             else if (Instruction1 == 3)
                                   txdData1=0;
                             break;
                    case 3:
                             txdData1=0;
                              if (Instruction1 == 1)
                                    TempSetpointHi = rxdData1;
                              else if  (Instruction1 == 2)
                                        HVSetpointHi1 = rxdData1;
                              break;
                    default:
                              CommandCounter1 = 0;
                              break;
                }  
        }

		//Now its Time to set HV1 and HV2//

        HVSetpoint1 = HVSetpointHi1;  //HV + setting
        HVSetpoint2 = HVSetpointLow1; //HV - setting
                // loading the pwm buffer for HV1 and HV2// 

        SetPulseOC1(0x0 , HVSetpoint1 +2 );
        SetPulseOC2(0x0 , HVSetpoint2 +2 );
                //Now its Time to set TempSetpoint1 and TempSetpoint2//

        TempSetpoint = TempSetpointHi;
        TempSetpoint = TempSetpointLow + (TempSetpoint * 0xff) ;


        if (TempSetpoint > 700) 
             TempSetpoint =700;    //limit for TempSetpoint     
                //Now its time to read ADCs //

        AD1CHS0 = 0x0000;//select AN0 HV-Monitor 1//
        AD1CON1bits.SAMP = 1; // start sampling  ADC1
        while (!AD1CON1bits.DONE);
        AD1CON1bits.DONE = 0;
        ResultHV = ADC1BUF0;
        ResultHV >>= 2; //adjust adc from 10 bit to 8 bit value//
        VoltageMonitorLow1 = ResultHV ;

        AD1CHS0 = 0x001e;//select AN30 ESI-1//
        AD1CON1bits.SAMP = 1; // start sampling  ADC1
        while (!AD1CON1bits.DONE);
        AD1CON1bits.DONE = 0;
        MyResult = ADC1BUF0;
        MyResult >>= 2; //adjust adc from 10 bit to 8 bit value//



//---------------------------------------------------------------------------------------------------------------------
	
        AD1CHS0 = 0x0002;  //select AN2 TC2//
        AD1CON1bits.SAMP = 1; // start sampling  ADC1
        while (!AD1CON1bits.DONE);
        AD1CON1bits.DONE = 0;
        ResultTemperature = ADC1BUF0;
        Temperature = (ResultTemperature * 43 )/50 ; //consider termocouple posiotion in source//

                    
    }//for while
   return 0;

}//for main




