//*******************************************************************
//    DESCRIPTION: 	Header file modulo gestione memoria edit
//    AUTHOR:
//    HISTORY:    
//********************************************************************

#ifndef __EMEM
#define __EMEM

#include "edt_mng.h"

// COSTANTI PER DEFINIRE l'HARDWARE	E2PROM
#define EE_SIZE			 		32768					// byte 
#define EE_PAGE_SIZE	 		32						// dim. pagina
#define EE_TOTAL_PAGES      	(EE_SIZE/EE_PAGE_SIZE)	// num. pagine

#define EE_CONFIG_PAGES			((sizeof(CONFIG)+EE_OPCODE_SIZE+EE_MAGIC_SIZE+EE_PAGE_SIZE-1)/EE_PAGE_SIZE)
#define EE_CONFIG_ADDRESS   	((EE_TOTAL_PAGES-EE_CONFIG_PAGES)*EE_PAGE_SIZE)

#define EE_PRESET_PAGES     	((sizeof(PRESET)+EE_PAGE_SIZE-1)/EE_PAGE_SIZE)
#define EE_CURRPRESET_ADDRESS	((EE_TOTAL_PAGES-EE_CONFIG_PAGES-EE_PRESET_PAGES)*EE_PAGE_SIZE)
#define EE_PRESET_ADDRESS(n) 	((n)*EE_PRESET_PAGES*EE_PAGE_SIZE)

// N.B. OPCODE e MAGIC sono posizionati nell'ultima pagina di CONFIG 
#define EE_OPCODE_SIZE        	1  // byte
#define EE_OPCODE_ADDRESS    	(EE_CONFIG_ADDRESS+sizeof(CONFIG))
#define EE_MAGIC_SIZE        	4  // byte
#define EE_MAGIC_ADDRESS     	(EE_OPCODE_ADDRESS+EE_OPCODE_SIZE)

#define EE_OPCODE_VALUE_READY   0
#define EE_OPCODE_VALUE_PRESET0 0x10
#define EE_OPCODE_VALUE_SETUP   0xf0

#define EE_MAGIC_VALUE          0x8149163e

#define MEMORY_NORMAL			0
#define MEMORY_PUFF				1

extern void EditMemoryInit(int initstate);
extern int  EditCheckMagic(void);
extern int  EditReadMagic(void);
extern void EditReadOpCode(int *opcode);
extern void EditWriteOpCode(int opcode);
extern void EditWriteMagic(unsigned value);
extern void EditReadConfig(CONFIG *config);
extern void EditWriteConfig(CONFIG *config);
extern void test_emem(void);
#endif
