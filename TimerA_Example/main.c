#include <msp430g2553.h>
#include <msp430.h>

#define PUSHB (0x0008);
#define LED   (0x0040);

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1DIR |= LED; //Green LED as output
    P1OUT  = 0x00; //Switch everything off

    //PUSH BUTTON AS INPUT WITH PULL-UP
    P1DIR &= ~PUSHB; //Set P1.3 as input (push-button)
    P1REN |= PUSHB;
    P1OUT |= PUSHB; //Pull-up enabled

    //TIMER0_A0 INTERRUPT EVERY HALF SECOND
    TACCR0  = 62500 - 1; //Set the compare register (similar to PR2 for TMR2 on 8-bit PIC uC)
    TACCTL0 = CCIE; //Enable interrupts for T0A comparator
    TACTL = TASSEL_2 + ID_3 + MC_1 + TACLR;
    /*
     * TASSEL_2 ->  Set SMCLK as Timer_A source clock
     * ID_3     ->  1:8 prescaler applied to TA0 source clock
     * MC_1     ->  UP counting mode
     * TACLR    ->  Clear T0A count
     */

    //PUSH BUTTON INTERRUPT
    P1IES |= PUSHB;  //Descending edge
    P1IFG &= ~PUSHB; //Clear Interrupt flag before enabling interrupt
    P1IE  |= PUSHB;  //Enable GPIO Interrupt on P1.3 (PUSH)

    //General Interrupts Enable on Status Register (SR)
    _BIS_SR(GIE);
    while(1){
    	_BIS_SR(LPM1); //Turn off MCLK and CPU while doing nothing :)
    }

	return 0;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void CCR0_ISR(void){
	/*no flag clearing procedure needed. CCR0 has only one source
	 * and flag is automatically cleared
	 */
	P1OUT ^= LED; //Toggle green LED (P1.6)
}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void){
	switch(P1IFG&BIT3){
		case BIT3:
			P1IFG   &= ~PUSHB; //Clear interrupt flag on PUSH input
			TACCTL0 ^= CCIE;   //Toggle Capture Mode for Timer0_A0
			TACTL   |= TACLR;  //Clear TIMER0_A0's current counter value
		default:
			P1IFG = 0x00;
	}
}
