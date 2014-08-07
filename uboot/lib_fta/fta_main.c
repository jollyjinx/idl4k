/*****************************************************************************

File name   :  fta_main.c

Description : init the fta drivers

 COPYRIGHT (C) FTA Communication technologies 2007.

Date               Modification                                          Name
----               ------------                                          ----
 07/04/08          Created                                               FR

*****************************************************************************/
/* --- includes ----------------------------------------------------------- */
#include "fta/definition.h"
#include "iboot/iboot.h"

#if defined(USE_DISPLAY)
static BOOL display = FALSE;
#endif

/* ========================================================================
   Name:        usbApp
   Description: This function is called to use the USB boot
   ======================================================================== */
void usbApp(void)
{
	char *arg[] = {"run", "bootcmd"};
	char *usb[] = {"usb","reset"};

	/* (re)Start the USB */
	do_usb(NULL, 0, 2, usb);

	/* Set the Bootargs */
	setenv("bootargs","");
	setenv("bootcmd","fatload usb 0 84000000 app.ub;bootm 84000000");

	/* Start the Factory Application */
	do_run (NULL, 0, 2, arg);

	/* We should not come back ... */
	while(1);
}

/* ========================================================================
   Name:        initFtaLib
   Description: Initialize all IO peripherals related to FTA
   ======================================================================== */
void initFtaLib(void)
{
	U32 flag = 0;
	I8 *env;
	I8 selftest  = SELFTEST_ON;
	I8 selfstart = SELFSTART_V2F;
	I8 *arg[]    = {"run", "bootcmd"};

	/* We are "saving" the current configuration, because it is not valid after a setenv() */
	env = getenv("selftest");
	if(env)
		selftest = env[0];

	env = getenv("selfstart");
	if(env)
		selfstart = env[0];


#if defined(USE_DISPLAY)
	/* Init the use of the video */
	init_display();
	setVideoPio();

	/* Display something on screen */
	init_osd();
	display = TRUE;
#endif

	splash_update(64);

	/* Go to the prompt (Do not start automatically any application) */
	if( (getKeyPressed() & FP_KEY_DOWN) || (selfstart == SELFSTART_UBOOT) ) {
		setenv("bootcmd", NULL);
		return;
	}

	splash_update(128);

	/* U-Boot Self test */
	if(selftest == SELFTEST_ON)
	{
		if( test_fta() ) {
			hang();
		}
		else {
			setenv("selftest","0");
			saveenv();
		}
	}

	if( getKeyPressed() & FP_KEY_POWER ) {
		usbApp();
	}

	splash_update(256);
	if(selfstart == SELFSTART_NFS) {
		/* Set the Kernel (DEV) as the main application */
		setenv("bootcmd","bootm 0x80000");
	}
	else {
		/* Set the V2F as the main application
		 * The address provided is in the RAM, so clear it to avoid confusion
		 * (in case of some remaining data)
		 */
		memset((void *)0x80801000, 0, sizeof(ssa_header_t));
		setenv("bootcmd","bootm 0x80801000");
	}

	/* Set the Loader as the main application */
	if( getKeyPressed() & FP_KEY_UP ) {
		setenv("bootcmd","bootm 0x2C0000");
	}

	/* Set the Loader as the main application */
	if (!eeprom_read(CFG_I2C_EEPROM_ADDR, EEPROM_SSD_REG, (unsigned char*)&flag, sizeof(flag))) {
		if(flag == FORCE_UPDATE_CODE) {
			setenv("bootcmd","bootm 0x2C0000");
		}
	}

	do_run (NULL, 0, 2, arg);
}

void termFtaLib(void)
{
#if defined(USE_DISPLAY)
	if(display)
	{
		/* Remove access to the display */
		term_graphics ();
		/* Restore the initial value of the Video Registers */
		term_display();
	}
#endif

}

