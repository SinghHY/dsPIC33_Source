# dsPIC33_Source
//---------------------------------------------------------------------------------------------------------------//
//                                                                                                               //
//              This program written for SOURCE and HSID board of IONICAXX to                                    //
//              control the HV and Temperature of HSID and Source                                                //
//              MehrdadT@ionics.ca         Date : Aug 12th, 2008       				    						 //	
//              Version : 2.0 based on dsPIC33  							    								 //	
//              comment: this version has a PID control parameter to control temperature with seperate           //
//                       thermocouple                                                                            //
//              latest update for Source May12 was regarding immidiate heater shut down after zero set point     //
//              latest update for HV+ and HV- seperate setting                                                   // 
//              HSID heat controller fixed   July30th 2009                                                       //        
//              Calibration fixed Aug5th 2009                                                                    //
//              Fuzzy control start Aug 26th and finished successfull on Aug 28th                                //
//              Probe id detection added, too.  Sep 2nd 2009                                                     //  
//              Added new commands to poll temperature and HV without setting them on Dec3rd                     //
//              Change and mod for HV monitor and Temperature reading also temp control on Jan13,2010            //
//              Open loop temp control Jan13,2010                                                                //   
//              The SPI will be reset if the instruction in serial communication isn't right(1,2,3,11,12)        // 
//              SPI polling modified to Interrupt and it works perfect Feb21,2010                                // 
//              The HV pwm Carrier FQ modified to get max HV Mar24                                               //                                                  
//              PID tune up for Source heaters May 20th,2010                                                     // 
//              Changed termo limit to 700 and pwm to 60000                                                      //  
//              Changed PWM's limit to no limit to get 500C on probe1 and probe2 July 15,2011                    //
//              Adding Soft Start to resolving CE and TUV certification issue (ramp up current slow!) Sep6th,2011//                                                                                      
//              Add limitation for HV PWM in order to protect HV power suplly                                    //
//              Change power limitation from 45000 to 60000 on heat control March 2013                           //
//              change for APPI mode
//              Added Probe ID and ESI voltage control for MicroSpray-Sept 2014 (VT) 
//---------------------------------------------------------------------------------------------------------------//