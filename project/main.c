#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"


// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!!

#define LED BIT6/* note that bit zero req'd for display */
#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8
#define SWITCHES 15

int color_state = 0;
int shape_state = 0;
int step_state = 2;

unsigned int black = 65535;
unsigned int white = 0;
unsigned int step_max = 60;



static char

switch_update_interrupt_sense(){
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);/* if switch down, sense up */
  return p2val;
}


void
switch_init()/* setup switch */{
  P2REN |= SWITCHES;/* enables resistors for switches */
  P2IE |= SWITCHES;/* enable interrupts from switches */
  P2OUT |= SWITCHES;/* pull-ups for switches */
  P2DIR &= ~SWITCHES;/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler(){
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;            //shows which button pressed last 
}

// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};
short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;


void wdt_c_handler(){
  static int secCount = 0;
  secCount ++;
  if (secCount >= 25) {/* 10/sec */
    secCount = 0;
    redrawScreen = 1;
  }
}



void update_shape();

void
reset_screen(){
    clearScreen(COLOR_BLUE);
    drawChar5x7(5,5,'P',white,black);
    drawChar5x7(5,15,'r',white,black);
    drawChar5x7(5,25,'o',white,black);
    drawChar5x7(5,35,'j',white,black);
    drawChar5x7(5,45,'e',white,black);
    drawChar5x7(5,55,'c',white,black);
    drawChar5x7(5,65,'t',white,black);
    drawChar5x7(5,75,'3',white,black);
}


void main(){
  P1DIR |= LED;/**< Red led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);              /**< GIE (enable interrupts) */
  clearScreen(COLOR_BLUE);

  
  while (1) {/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;/* led off */
    or_sr(0x10);/**< CPU OFF */
    P1OUT |= LED;/* led on */
   }
}


void
update_shape(){
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 0, green = 0, red = 31;
  static unsigned char step =0;
  
  if(switches & SW4) step = step_max; //switch 4 resets the shape
  
  if (step <= step_max) {
    int startCol = col - step;
    int endCol = col + step;
    int startCol2 =  (screenWidth/2) -step_max + step;
    int endCol2 = (screenWidth/2) + step_max - step;
    int width1 = 1 + endCol - startCol;
    int width2 = 1 + (endCol2 - startCol2);
    
    
    

    
    if(step_state > 2)step_state = 0;
    if(shape_state> 2)shape_state = 0;
    if(color_state > 4) color_state = 0;
    // a color in this BGR encoding is BBBB BGGG GGGR RRRR
    unsigned int color = (blue << 11) | (green << 5) | red;
    if (switches & SW3){//switch 3 changes size of shape in states
      step_state +=1;
      step = 0;
      reset_screen();
    }
    if(switches & SW1){   //switch 1 changes color in states
      color_state +=1;
      step = 0;
      reset_screen();
    }
    if (switches & SW2){// switch 2 changes the shape drawn
      shape_state += 1;
      step =0;
      reset_screen();
    }

    //shape size switch
    switch(step_state){
    case 0:
      step_max = 10;
      break;
    case 1:
      step_max = 30;
      break;
    case 2:
      step_max = 60;
      break;
    default:
      step = 0;
      reset_screen();
      break;
    }
    //shape drawn switch
    switch(shape_state){
    case 0://draws hourglass
      fillRectangle(startCol, row+step, width1, 1, color);
      fillRectangle(startCol, row-step, width1, 1, color);
      break;
    case 1://draws diamond
      fillRectangle(startCol2, row+step, width2, 1, color);
      fillRectangle(startCol2, row-step, width2, 1, color);
      break;
    case 2:
      drawRectOutline(col-(step/2),row-(step/2),step+1,step+1,color);
      break;
    default:
      reset_screen();
      break;
    }
    //color drawn switch
    switch(color_state){
    case 0:
      red = 31;
      green = 0;
      blue = 0;
      break;
    case 1:
      red = 0;
      green = 31;
      blue = 0;
      break;
    case 2:
      red = 31;
      green = 31;
      blue = 0;
      break;
    case 3:
      red = 31;
      blue = 31;
      green = 0;
      break;
    default:
      red = 0;
      blue = 31;
      green = 0;
      break;
    }
    step ++;
  } else {
    reset_screen();
    step =0;
  }
}




/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;      /* clear pending sw interrupts */
    switch_interrupt_handler();/* single handler for all switches */
  }
}
