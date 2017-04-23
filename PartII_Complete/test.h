#ifndef __TEST_H__
#define __TEST_H__

void delay(unsigned long);
void testfastlines(unsigned int, unsigned int);
void testdrawrects(unsigned int);
void testfillrects(unsigned int, unsigned int);
void testfillcircles(unsigned char, unsigned int);
void testdrawcircles(unsigned char, unsigned int);
void testtriangles();
void testroundrects();
void testlines(unsigned int);
void lcdTestPattern(void);
void lcdTestPattern2(void);

void testPrint(int x, int y, char* str, unsigned int color, unsigned int bg, unsigned char size);

#endif //  __TEST_H__
