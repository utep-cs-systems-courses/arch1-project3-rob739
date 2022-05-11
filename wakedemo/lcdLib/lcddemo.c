/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);

  drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

  fillRectangle(30,30, 60, 60, COLOR_ORANGE);

  
 int  col = 30;
 int  row = 30;
  
  for (int j = 0; j < 50; j++) {
    drawPixel(col,row, COLOR_RED);
    drawPixel(col,80-row, COLOR_WHITE);
      col ++;
      row ++;
  }
}
