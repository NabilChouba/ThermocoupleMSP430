
// board MSP-FET430U64
// chip MSP430F423A
#include "lcd.h"
#include "msp430x42x.h"
//#include "intrinsics.h"


float Cof500  [9] = { 25.08355, 0.07860106, -0.2503131, 0.08315270, -0.01228034, 0.0009804036, -0.00004413030, 0.000001057734, -0.00000001052755 } ;
float Cof1300 [7] = { -131.8058, 48.30222, -1.646031, 0.05464731, -0.0009650715, 0.000008802193, -0.00000003110810 };
float CofRef  [8] = { -0.017600413686, 0.038921204975, 0.000018558770032, -0.00000099457592874, 0.00000000031840945719, -0.00000000000056072844889, 0.00000000000000056075059059, -0.00000000000000000032020720003 } ;


float  expfloat(float x, int n)  
{
 float r=1.0;
 for (int i=0;i<n;i++)
   r*=x;
 return(r);
   
}

// table that contain the canne Temperature for curve display  
char dT[20];

int col, row;
int kcol = 0;
char buttons[16] = "123A456B789C*0#D";
char pressed[6] = "      ";
char pressed_cur =0;
#define KCOL1 BIT1//Keypad Column 1
#define KCOL2 BIT2//Keypad Column 2
#define KCOL3 BIT3//Keypad Column 3
#define KCOL4 BIT4//Keypad Column 4

// *** OUTPUT ***
#define KLINE1 BIT3 // P2
#define KLINE2 BIT4 // P2
#define KLINE3 BIT5 // P2
#define KLINE4 BIT0 // P1


void initKeypad( ){

  P1DIR |= BIT0;                     // Set Lines to output direction
  P2DIR |= BIT3 | BIT4 | BIT5 ;      // Set Lines to output direction
  
  P1OUT  &= ~ BIT0;                 // Set the pins to low
  P2OUT  &= ~(BIT3 | BIT4 | BIT5);  // Set the pins to low

  P1DIR &= ~(BIT1 | BIT2 | BIT3 | BIT4);  // Set Columns to input direction
 
  //P1REN = (KCOL1 | KCOL2 | KCOL3 | KCOL4);    // Set the pullup resistors for the inputs
  
  P1IES = 0;     // Set interrupts to enable when line goes high (L2H edge)
  P1IE  = (BIT1 | BIT2 | BIT3 | BIT4);

  // Set the ROWS to high
  P1OUT  |=  BIT0;                 // Set the pins to low
  P2OUT  |= (BIT3 | BIT4 | BIT5);  // Set the pins to low

  __enable_interrupt(); // TI code to valid configuration

  do 
  {
    P1IFG = 0;
  } while (P1IFG != 0);

}



// ISR for colum interrupts
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR(void)
{
  P1IE = 0; // Disable more interrupts while processing
  // Determine which column triggered the interrupt
  if (P1IFG & KCOL1)
  {
    col = 0;
    kcol = KCOL1;
  } 
  else if (P1IFG & KCOL2)
  {
    col = 1;
    kcol = KCOL2;
  }
  else if (P1IFG & KCOL3)
  {
    col = 2;
    kcol = KCOL3;
  }
  else if (P1IFG & KCOL4)
  {
    col = 3;
    kcol = KCOL4;
  }
  
  // Scan rows by setting them to high one by one
  row = 5;
  
  P1OUT ^= KLINE4;  
  if (!(P1IN & kcol) && row==5) row =3;
  P2OUT ^= KLINE3;
  if (!(P1IN & kcol)&& row==5) row =2;
  P2OUT ^= KLINE2;
  if (!(P1IN & kcol)&& row==5)row =1;
  P2OUT ^= KLINE1;
  if (!(P1IN & kcol)&& row==5) row =0;
  
  // Get the pressed key
  
  pressed [pressed_cur]= buttons[(4*row)+ col];
 
  if (pressed [pressed_cur] =='D') {
    for (pressed_cur=0;pressed_cur<6;pressed_cur++)
       pressed [pressed_cur]= ' ';
  pressed_cur=0;
  } else pressed_cur++;
    
  LCD_TXT(pressed,0,0,6);

  // Restore interrupts
  P2OUT ^= KLINE1 | KLINE2 | KLINE3;
  P1OUT ^= KLINE4 ;
    
  do 
  {
    P1IFG = 0;
  } while (P1IFG != 0);

  P1IE  = (KCOL1 | KCOL2 | KCOL3 | KCOL4);
}


int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  int i; 
  for(i = 0; i < 0xFFFF; i++){}

  // time TA_0 ISR. Toggles every 50000 SMCLK cycles. 
  // Default DCO frequency used for TACLK. During the TA_0
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 50000;
  TACTL = TASSEL_2 + MC_2;                  // SMCLK, continuous mode
 
  
  //all as input
  P1DIR = 0x0;
  P2DIR = 0x0;
  
  P1SEL = 0x0;
  P2SEL = 0x0;

  // Mi        (PIN49)
  //no need to use
  P1DIR |= BIT4;
  //P1OUT &= ~BIT4;
  P1OUT |= BIT4;
  

  //LCD init
  LCDInit();
  //LCD Clear
  LCDClear();
  
  initKeypad( ) ;
  
//while (1)  ;
  //image dispay 
  // LCD_IMG( IMG_ALL,95,1,11,6);   
  
  
 
  //init temperature table
   for (int y=0;y<60;y++) 
     dT[y]=0;



                                            // by compiler optimization

 // WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
 // FLL_CTL0 |= XCAP14PF;                     // Configure load caps
 // for (i = 0; i < 10000; i++);              // Delay for 32 kHz crystal to
                                            // stabilize

  SD16CTL = SD16REFON+SD16SSEL0;            // 1.2V ref, SMCLK
  // canne Temperature 
  SD16CCTL0 |= SD16SNGL ;                   // Single conv
  SD16INCTL0 |= SD16GAIN_32 + SD16INCH0;
  //onchip sensor Temperature
  SD16CCTL2 |= SD16SNGL ;            // Single conv, enable interrupt
  SD16INCTL2 |= SD16INCH_6;                 // Select Channel A6

  for (i = 0; i < 0x3600; i++);             // Delay for 1.2V ref startup

  //get  : onchip sensor Temperature 
  //original source file : fet410_sd16_06
  float Tref =0.0;
  for(int rep=0;rep<100;rep++){              // take 100 sample and get the avrage value
     SD16CCTL2 |= SD16SC;                    // Set bit to start conversion
     while ((SD16CCTL2 & SD16IFG)==0);       // Poll interrupt flag for CH2
     
     Tref += ((unsigned long)SD16MEM2 * 909)/65536 - 727;

    }
    Tref /=100; //avraging 
 
  // get Vref that depend from the current temperature 
  // conversion from the Temerature to mv 
  float Vref = CofRef[0] + CofRef[1]*Tref + CofRef[2]*expfloat(Tref,2) + CofRef[3]*expfloat(Tref,3) + CofRef[4]*expfloat(Tref,4) + CofRef[5]*expfloat(Tref,5) + CofRef[6]*expfloat(Tref,6) + CofRef[7]*expfloat(Tref,7) ;
  
  
  while (1) {
    

    int count= 0;
  
    float T = 0.0;      // canne temperature
    float Vp=0.0;       // canne Volatage
  
    for(int rep=0;rep<100;rep++){             // take 100 sample and get the avrage value
      SD16CCTL0 |= SD16SC;                    // Set bit to start conversion
      while ((SD16CCTL0 & SD16IFG)==0);       // Poll interrupt flag for CH2
     
      // Vref used for the ADC SD is Vrer= 0.6
      // 0x8000 is the value Zero of the ADC
      // 900 is the found offset in our case
      // 32 is the PGA (gain) value
      // *1000.0 value are in mv 
      Vp+= Vref + (0.6*1000.0/32767) * (((SD16MEM0*1.0-0x8000) -900)/32) ;
    }
    Vp /=100;   //avraging  
    
    // conversion from the mv to Temerature
    if (Vp < 20.644)
      T =  Cof500[0]*Vp + Cof500[1]*expfloat(Vp,2) + Cof500[2]*expfloat(Vp,3) + Cof500[3]*expfloat(Vp,4) + Cof500[4]*expfloat(Vp,5) + Cof500[5]*expfloat(Vp,6) + Cof500[6]*expfloat(Vp,7) + Cof500[7]*expfloat(Vp,8) + Cof500[8]*expfloat(Vp,9);
    else 
      T = Cof1300[0] + Cof1300[1]*Vp + Cof1300[2]*expfloat(Vp,2) + Cof1300[3]*expfloat(Vp,3) + Cof1300[4]*expfloat(Vp,4) + Cof1300[5]*expfloat(Vp,5) + Cof1300[6]*expfloat(Vp,6);
/*
   // convert value to String
      char TPross[2];
      char TFour[4];
      TPross[0]=  (int)(Tref/10) ;
      TPross[1]=  (int)(Tref) - TPross[0]*10 + '0';
      TPross[0]+= '0'; 
      
      TFour[0]=  (int)(T/1000) ;
      TFour[1]=  (int)(T/100) - TFour[0]*10 ;
      TFour[2]=  (int)(T/10)  - TFour[0]*100  - TFour[1]*10 ;
      TFour[3]=  (int)(T/1)   - TFour[0]*1000 - TFour[1]*100 - TFour[2]*10 ;
      TFour[0]+= '0';  TFour[1]+= '0';  TFour[2]+= '0';  TFour[3]+= '0'; 
      

    // send value to the LCD
      LCD_TXT("T=",85,7,2);
      LCD_TXT(TPross,95,7,2);
      LCD_TXT("TFour=",0 ,7,6);
      LCD_TXT(   TFour,40,7,4);
  */
      //if (count%10 == 0) {dT[count/10]=T;if (count>=600) count=0;}

  //   dT[count]=(char)T/5;
   //     dT[count]=(char)count;
   // LCD_DrowGraf(dT);
      
            count++;
//            if (count>60) count=0;
      
     }
}


// ":)" or ":(" for debug
int flip=0;
// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
  
  flip++;
  if (flip%2) 
    LCD_TXT(":(",65,7,2);
  else 
    LCD_TXT(":)",65,7,2);
    
  CCR0 += 50000;                            // Add offset to CCR0
}
