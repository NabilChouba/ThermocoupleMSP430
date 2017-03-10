
#include "lcd.h"
#include "msp430x42x.h"

/****************************************************************************/
/*  Move internal LCD curssor                                               */
/*  Function : LCDSend                                                      */
/*      Parameters                                                          */
/*          Input   :  x = colum , y = row                                  */
/*          Output  :  Nothing                                              */
/****************************************************************************/

void gotoxy(unsigned char x, unsigned char y)     {
     // y can be 0 .. 8 (step by 8bit block)
     // x can be 0 ..130 
     
       LCDSend( 0xB0+y, SEND_CMD ); // page 0
       LCDSend( 0x10+x/16, SEND_CMD ); // colum msb 0 (no need there is only one line)
       LCDSend( 0X00+x%16, SEND_CMD ); // colum lsb 0
}

    
/*void LCD_IMG(int c,char col,char row,char colWight,char rowWight) {

  for (char page=0;page<rowWight;page++) {
    gotoxy(0+col, page+row);
    for (char column=0;column<colWight;column++) {
        LCDSend(ImageAll[colWight*page+column],SEND_CHR);
    }
  }
}*/

    


//draw graf
int tabGraf[9]={0,BIT7,BIT7|BIT6,BIT7|BIT6|BIT5,BIT7|BIT6|BIT5|BIT4,BIT7|BIT6|BIT5|BIT4|BIT3,BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1,BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0};

//allow you to draw a simple line
//int tabGraf[9]={0,BIT7,BIT6,BIT5,BIT4,BIT3,BIT2,BIT1,BIT0};

//from colPixel we will draw the graf :
//go by block and draw the colone if needed
int translate(char colPixel,char y){
  // 6 is the number of page used 7 (-1) in this case ..
  int n = colPixel - (6-y) *8;
  if ( n<0 )
    return (0x00);
  if (n>8)
    return (0xFF);
   //return (0x00);//allow you to draw a simple line
  else 
    return tabGraf[n];
}


void LCD_DrowGraf(char d[])
	{
		int i,j;
                for (j=0;j<7;j++){
                // graf is composed from 60 points  
		gotoxy(0,j);
                for (i=0;i<60;i++)
                  LCDSend(translate(d[i],j),1);
                
                }
		//delay_ms(d);
	}



void LCDClear() {

  for (char page=0;page<8;page++) {
    gotoxy(0, page);
    for (char column=0;column<129;column++) {
      LCDSend(0x00,SEND_CHR);                  
    }
  }
}

void table_to_nokialcd(char charsel)               // extract ascii from tables & write to LCD
   {
     int charpos;
     char chardata;
     
      if (charsel<0x20)return;            // 5 bytes
      if (charsel>0x7f)return;
      for (int char_row=0;char_row<5;char_row++)
         {
            if (charsel<0x50)
               {
                  charpos=(((charsel&0xff)-0x20)*5);
                  chardata=TABLE5[(charpos+char_row)];
               }                           // use TABLE5
            else if (charsel>0x4f)
               {
                  charpos=(((charsel&0xff)-0x50)*5);
                  chardata=TABLE6[(charpos+char_row)];  
               }                           // use TABLE6
            LCDSend(chardata,1);    // send data to nokia
           }
      LCDSend(0X00,1);               // 1 byte (always blank)
   }


void LCD_TXT(char a[30],unsigned char x,unsigned char y,int d)
	{
		gotoxy(x,y);
                for(int i = 0 ;i<d ;i++)
		  table_to_nokialcd(a[i]);
		//delay_ms(d);
	}



   
// simple delay
void Delay(unsigned long a) { while (--a!=0); }


/****************************************************************************/
/*  Send to LCD                                                             */
/*  Function : LCDSend                                                      */
/*      Parameters                                                          */
/*          Input   :  data and  SEND_CHR or SEND_CMD                       */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCDSend(unsigned char data, unsigned char cd) {

  // Enable display controller (active low).
  P2OUT &= ~ BIT0 ; // STE0 (CE)

  // command or data
  if(cd == SEND_CHR) { 
      P1OUT |= BIT7 ;  // SOMI0 (D/S)
  }
  else {//SEND_CMD put 0
     P1OUT &= ~BIT7 ;  // SOMI0 (D/S)
  }

  ///// SEND SPI /////

  //send data
  U0TXBUF = data;

  //Wait for ready U0TXBUF
  while((U0TCTL & TXEPT) == 0);

  // Disable display controller.
  P2OUT |= BIT0 ; // STE0 (CE)

}

/****************************************************************************/
/*  Init LCD Controler                                                      */
/*  Function : LCDInit                                                      */
/*      Parameters                                                          */
/*          Input   :  Nothing                                              */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCDInit(void)
{

  //  Pull-up on reset pin.
  P1OUT |= BIT5;

  // Pin configuration - all as output
  P2DIR |= BIT0 ; // STE0   CS1B    (PIN45)
  P1DIR |= BIT6 ; // SIMO0  SID-DB7 (PIN47)
  P1DIR |= BIT7 ; // SOMI0  RS      (PIN46) (D/S)
  P2DIR |= BIT1 ; // ULCK0  SCLK-DB6(PIN44) 
  P1DIR |= BIT5 ; // RES   RESETB   (PIN48)

  // Pin select function
  P2SEL &= ~ BIT0 ; // STE0
  P1SEL |= BIT6 ;   // SIMO0
  P1SEL &= ~ BIT7 ; // SOMI0 (D/S)
  P2SEL |= BIT1 ;   // ULCK0
  P1SEL &= ~ BIT5 ; // RES
  


  //  Toggle display reset pin.
  P1OUT &=~ BIT5;
  Delay(10000);
  P1OUT |= BIT5;

  // Init SPI
  U0CTL   = 0x16;   // SPI Mode, 8bit, Master mode
  U0TCTL  = 0xB2;   // 3pin Mode, clock->SMCLK, no CKPL (poliarity), no CKPH (phase)
  //U0TCTL  = 0x30;   // 4pin Mode, clock->SMCLK, no CKPL (poliarity), no CKPH (phase)

  //U0BR0   = 0x2A;   // 19200 -> 19200 = ~800Khz/46
  U0BR0   = 0x02;   // 19200 -> 19200 = ~800Khz/46
  //U0BR0   = 0x15;     // 38000 -> 38000 = ~800Khz/21
  U0BR1   = 0x00;
  UMCTL0  = 0x00;   // in spi mode don't used

  ME1     = 0x40;   // Enable SPI0
  //ME2     = 0x01;   // Enable SPI0

  // Disable display controller.
  P2OUT |= BIT0 ; // STE0

  Delay(100);

  // Send init sequence of command
  // 37 page : S6B0724
  //           132 SEG / 65 COM DRIVER & CONTROLLER FOR STN LCD
  
  //dt_lcd_write_command(0xe2);	// reset
  LCDSend( 0xe2, SEND_CMD );
	
  //dt_lcd_write_command(0xa0);	// ADC select: normal
  LCDSend( 0xa0, SEND_CMD );
  
  //dt_lcd_write_command(0xc0);	// SHL select:
  LCDSend( 0xc8, SEND_CMD );//8 or 0 
  
  //dt_lcd_write_command(0xb2);	// LCD bias select:
  LCDSend( 0xb2, SEND_CMD );
  
  //dt_lcd_write_command(0x2c);	// Voltage converter ON
  LCDSend( 0x2c, SEND_CMD );
  //delay_ms(1);
  Delay(0xFFFF);
  //dt_lcd_write_command(0x2e);	// Voltage regulator ON
  LCDSend( 0x2e, SEND_CMD );
  //delay_ms(1);
  Delay(0xFFFF);
  //dt_lcd_write_command(0x2f);	// Voltage follower ON
  LCDSend( 0x2f, SEND_CMD );
  
  //dt_lcd_write_command(0x25);	// Regulator resistor select 0x20..0x27
  LCDSend( 0x25, SEND_CMD );
  
  //dt_lcd_write_command(0x81);	// Reference voltage register set
  LCDSend( 0x81, SEND_CMD );
  
  //dt_lcd_write_command(0x18);	// Reference voltage 0..63
  LCDSend( 0x18, SEND_CMD );

  //dt_lcd_write_command(0x40);	// initial display line 0x40..0x7f
  LCDSend( 0x40, SEND_CMD );
  
  //dt_lcd_write_command(0xaf);	// display ON
  LCDSend( 0xaf, SEND_CMD );
}

/****************************************************************************/
/*  Set LCD Contrast                                                        */
/*  Function : LcdContrast                                                  */
/*      Parameters                                                          */
/*          Input   :  contrast                                             */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCDContrast(unsigned char contrast) {

    //  LCD Extended Commands.
    LCDSend( 0x21, SEND_CMD );

    // Set LCD Vop (Contrast).
    LCDSend( 0x80 | contrast, SEND_CMD );

    //  LCD Standard Commands, horizontal addressing mode.
    LCDSend( 0x20, SEND_CMD );
}


/****************************************************************************/
/*  Send string to LCD                                                      */
/*  Function : LCDStr                                                       */
/*      Parameters                                                          */
/*          Input   :  size text, text                                      */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCDStr(unsigned char size, unsigned char *dataPtr ) {

  // loop to the and of string
  while ( *dataPtr ) {
    LCDChr( size, *dataPtr++ );
  }
}



