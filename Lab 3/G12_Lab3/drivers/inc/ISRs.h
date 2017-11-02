#ifndef _ISRS
#define _ISRS

	// extern volatile int hps tim0 int flag;	// from example

	extern void A9_PRIV_TIM_ISR();					// given
	extern void HPS_GPIO1_ISR();
	extern void HPS_TIM0_ISR();
	extern void HPS_TIM1_ISR();
	extern void HPS_TIM2_ISR();
	extern void HPS_TIM3_ISR();
	extern void FPGA_INTERVAL_TIM_ISR();
	extern void FPGA_PB_KEYS_ISR();
	extern void FPGA_Audio_ISR();
	extern void FPGA_PS2_ISR();
	extern void FPGA_JTAG_ISR();
	extern void FPGA_IrDA_ISR();
	extern void FPGA_JP1_ISR();
	extern void FPGA_JP2_ISR();
	extern void FPGA_PS2_DUAL_ISR();

	extern int hps_tim0_int_flag;					// written by us
	extern int pb_int_flag;							// written by us

#endif
