//*******************************************************************
//
// DESCRIPTION: Modulo gestione i2c bus
// AUTHOR: 		
//
// HISTORY:    
//
//*******************************************************************

#include "LPC23XX.h"
#include "config.h"
#include "i2c.h"

static int page_size;
static int eeprom_cs;

//-----------------------------------------------------------------------------

static int m24xx_page_size(int eeprom_type, int eeprom_addr,int * pg_sz);
int m24xx_set_addr(
	int eeprom_type,    // EEPROM type
    int eeprom_addr,    // start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,  // Hardware A2,A1,A0 (valid from 24XX32)
    int * ctrl_val,     // Value of ctrl(return)
    int * addr_hi_val,  // Value of addr_hi(return)
    int * addr_lo_val); // Value of addr_lo(return)

static int i2c_lpc_m24xx_wr(
    int eeprom_addr,  	// start eeprom addr ( not included Hardware A2,A1,A0)
    char * buf,         // Data srs buf
    int num );          // Bytes to write qty

static int m24xx_write(
    int eeprom_addr,    // start eeprom addr ( not included Hardware A2,A1,A0)
    char * buf,         // Data src buf
    int num);           // Bytes to write qty

static int m24xx_read(
  	int eeprom_addr,    // start eeprom addr ( not included Hardware A2,A1,A0)
  	char * buf,         // Data dst buf
  	int num);           // Bytes to read qty

//===========================================================================
//================ Common routines ==========================================
//===========================================================================

void i2c_lpc_init(int Mode)
{
	PCONP |= PCI2C1;

  	PINSEL1 |=  PINSEL1_P0_20_SCL1 | PINSEL1_P0_19_SDA1;	// set P0.19 and P0.20 to I2C1 SDA and SCK 
															// function to 11 on both SDA and SCK. 

	if(Mode == I2C_SPEED_400)
   	{
    	I21SCLH = 47;  
      	I21SCLL = 93;  
   	}
   	else //Slow
   	{
      	I21SCLH = 47*4;
      	I21SCLL = 93*4;
   	}

   	I21CONCLR = 0xFF;           // Clear all flags
   	I21CONSET = 0x40;           // Set Master Mode
   	I21CONSET |= I2C_FLAG_I2EN; // Enable I2C

	m24xx_page_size(EEPROM_24XX256, 0,&page_size);
	eeprom_cs = EEPROM_CS_VAL;
}

//---------------------------------------------------------------------------
static void i2c_lpc_wr_byte(int byte)
{
   	I21DAT = byte;
   	I21CONCLR = I2C_FLAG_SI;                // Clear SI
   	while(!(I21CONSET & I2C_FLAG_SI));      // End wr POINT
}

//---------------------------------------------------------------------------
static void i2c_lpc_stop(void)
{
    //-- Set STOP condition
   	I21CONCLR = I2C_FLAG_SI;                  // Clear SI
   	I21CONSET |=  I2C_FLAG_AA | I2C_FLAG_STO; // Clear NO ASK
}

//---------------------------------------------------------------------------
static int i2c_lpc_ctrl(int ctrl)
{
   	unsigned chk;
   	//-- Set START
   	I21CONCLR = 0xFF; // Clear all bits
   	I21CONSET |= I2C_FLAG_I2EN | I2C_FLAG_STA;
   	while(!(I21CONSET & I2C_FLAG_SI));      // End START
   	//-- Set ADDRESS
   	I21DAT = ctrl;
   	I21CONCLR = I2C_FLAG_STA | I2C_FLAG_SI; // Clear START & SI
   	if(ctrl & 1) //-- RD
      	chk = 0x40; //-- 40H - SLA+R has been transmitted; ACK has been received
   	else
      	chk = 0x18; //-- 18H - SLA+W has been transmitted; ACK has been received
   	while(!(I21CONSET & I2C_FLAG_SI));      //-- End CTRL
   	if(I21STAT != chk)
   	{
      	i2c_lpc_stop();
      	return I2C_ERR_NO_RESPONSE;
   	}
   	return I2C_NO_ERR;
}

//---------------------------------------------------------------------------
static int i2c_lpc_rx_to_buf(char * buf,int num)
{
	int rc;

   	if(buf == NULL)
      	return I2C_ERR_WRONG_PARAM;

   	rc = num;
   	if(rc > 1)
   	{
  		I21CONCLR = I2C_FLAG_SI;
      	I21CONSET |= I2C_FLAG_AA;
      	for(;;)
      	{
        	while(!(I21CONSET & I2C_FLAG_SI));  // End Data from slave;
         	*buf++ = (unsigned char)I21DAT;
         	rc--;
         	if(rc <= 0)
            	break;
         	else if(rc == 1)
            	I21CONCLR = I2C_FLAG_AA | I2C_FLAG_SI;  // After next will NO ASK
         	else
         	{
            	I21CONCLR = I2C_FLAG_SI;
            	I21CONSET |= I2C_FLAG_AA;
         	}
      	}
   	}
   	else if(rc == 1)
   	{
     	I21CONCLR = I2C_FLAG_AA | I2C_FLAG_SI;  // After next will NO ASK
      	while(!(I21CONSET & I2C_FLAG_SI));  // End Data from slave;
      	*buf = (unsigned char)I21DAT;
   	}
   	else //err
      	return I2C_ERR_WRONG_PARAM;

   	return I2C_NO_ERR;
}
//----------------------------------------------------------------------
static int i2c_lpc_ask_polling_op(int ctrl)  //-- wait until write finished
{
   	int rc;
   	int i;

   	for(i=0;i < I2C_WR_24XX_TIMEOUT; i++) // actually wr = ~110 but timeout =10000
   	{
   		I21CONSET = I2C_FLAG_STA;
      	I21CONCLR = I2C_FLAG_SI;  // Here - clear only SI (not all rI2C_I2CONCLR)
      	while(!(I21CONSET & I2C_FLAG_SI));        //wait the ACK

     	I21DAT = ctrl & 0xFE;; // R/WI = 0
      	I21CONCLR = I2C_FLAG_SI | I2C_FLAG_STA; // Clear START & SI
     	while(!(I21CONSET & I2C_FLAG_SI));	// wait the ACK
      	rc = I21STAT;
      	if(rc == 0x18) // got ACK after CLA + W
         	break;
   	}
   	if(i == I2C_WR_24XX_TIMEOUT)
      	return I2C_ERR_24XX_WR_TIMEOUT;
   	return I2C_NO_ERR;
}
//===========================================================================
//================ EEPROM 24XX series =======================================
//===========================================================================

//----------------------------------------------------------------------------
static int m24xx_page_size(int eeprom_type, int eeprom_addr,int * pg_sz)
{
   	int page_sz,rc;

   	page_size = 0;
   	rc = I2C_NO_ERR;
   	switch(eeprom_type)
   	{
      	case EEPROM_24XX04:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX04)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 16; // 24LC04B have page size = 16,24C04A = 8
        	break;
      	case EEPROM_24XX08:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX08)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 16;
         	break;
      	case EEPROM_24XX16:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX16)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 16;
         	break;
      	case EEPROM_24XX32:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX32)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 32;
         	break;
      	case EEPROM_24XX64:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX64)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 32;
         	break;
      	case EEPROM_24XX128:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX128)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 64;
         	break;
      	case EEPROM_24XX256:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX256)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 64;
         	break;
      	case EEPROM_24XX512:
         	if(eeprom_addr > EEPROM_MAX_ADDR_24XX512)
            	rc = I2C_ERR_24XX_BAD_ADDR;
         	else
         		page_sz = 128;
         	break;
   	}
   	if(rc != I2C_NO_ERR)
      	return rc;
   	if(page_sz == 0)  // Bad eeprom_type
      	return I2C_ERR_24XX_BAD_TYPE;

   	if(pg_sz)
      	*pg_sz = page_sz;
   	return I2C_NO_ERR;
}
//----------------------------------------------------------------------------
int m24xx_set_addr(
    int eeprom_type,     //-- EEPROM type
    int eeprom_addr,     //-- start eeprom addr ( not included Hardware A2,A1,A0)
    int eeprom_cs_val,   //-- Hardware A2,A1,A0 (valid from 24XX32)
    int * ctrl_val,      //-- Value of ctrl(return)
    int * addr_hi_val,   //-- Value of addr_hi(return)
    int * addr_lo_val)   //-- Value of addr_lo(return)
{
   	int ctrl;
   	int addr_hi;
   	int addr_lo;
   	int rc;

   	rc = I2C_NO_ERR;
   	ctrl = 0;
   	addr_hi = 0;
   	addr_lo = 0;

   	switch(eeprom_type)
   	{
      	case EEPROM_24XX04:   // 24LC04B ignore AO,A1,A2 pins


         	ctrl = (eeprom_addr>>7) & 0x02; //-- 00000010
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = eeprom_addr & 0xFF;
         	addr_lo = -1;
         	break;

      	case EEPROM_24XX08: // 24LC08B ignore AO,A1,A2 pins

         	ctrl = (eeprom_addr>>7) & 0x06; //-- 00000110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = eeprom_addr & 0xFF;
         	addr_lo = -1;
        	break;

      	case EEPROM_24XX16: // 24LC16B ignore AO,A1,A2 pins

         	ctrl = (eeprom_addr>>7) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = eeprom_addr & 0xFF;
         	addr_lo = -1;
         	break;

      	case EEPROM_24XX32:

         	ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = (eeprom_addr>>8) & 0x0F;
         	addr_lo = eeprom_addr & 0xFF;
         	break;

     	case EEPROM_24XX64:

         	ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = (eeprom_addr>>8) & 0x1F;
         	addr_lo = eeprom_addr & 0xFF;
         	break;

      	case EEPROM_24XX128:

         	ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = (eeprom_addr>>8) & 0x3F;
         	addr_lo = eeprom_addr & 0xFF;
         	break;

      	case EEPROM_24XX256:

         	ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = (eeprom_addr>>8) & 0x7F;
         	addr_lo = eeprom_addr & 0xFF;
         	break;

      	case EEPROM_24XX512:

         	ctrl = (eeprom_cs_val<<1) & 0x07; //-- 00001110
         	ctrl |= 0xA0; //-- 1010xxxx
         	addr_hi = (eeprom_addr>>8) & 0xFF;
         	addr_lo = eeprom_addr & 0xFF;
         	break;
   	}
   	if(rc != I2C_NO_ERR)
      	return rc;
   	if(ctrl == 0)
      	return I2C_ERR_24XX_BAD_TYPE;

   	if(ctrl_val)
      	*ctrl_val = ctrl;
   	if(addr_hi_val)
      	*addr_hi_val = addr_hi;
   	if(addr_lo_val)
      	*addr_lo_val = addr_lo;

   	return I2C_NO_ERR;
}

//----------------------------------------------------------------------------
static int i2c_lpc_m24xx_wr(
     int eeprom_addr,    //-- start eeprom addr ( not included Hardware A2,A1,A0)
     char * buf,         //-- Data srs buf
     int num )           //-- Bytes to write qty
{
   	int rc;
   	int ctrl;
   	int addr_hi;
   	int addr_lo;
	
	/*
   	rc = m24xx_set_addr(eeprom_type,eeprom_addr,eeprom_cs_val,
                                  &ctrl,&addr_hi,&addr_lo);
   	if(rc != I2C_NO_ERR)
      	return rc;
	*/
	// *****VALE SOLO PER 24LC256 !!!!!!!!
	ctrl = (eeprom_cs<<1) & 0x07; //-- 00001110
 	ctrl |= 0xA0; //-- 1010xxxx
 	addr_hi = (eeprom_addr>>8) & 0x7F;
 	addr_lo = eeprom_addr & 0xFF;
	// ********************************* 

   	//--- wr START + CONTROL
   	rc = i2c_lpc_ctrl(ctrl); //-- Now WR (RD/WI = 0)
   	if(rc != I2C_NO_ERR)
      	return rc;
   	//--- wr ADDRESS
   	i2c_lpc_wr_byte(addr_hi);
   	if(addr_lo != -1)
      	i2c_lpc_wr_byte(addr_lo);
   	//---  Write  data
   	while(num--)                 //-- transmit data until length>0
   	{
      	rc = *buf++; //---
      	i2c_lpc_wr_byte(rc);
   	}
   	//-----------------------
   	i2c_lpc_stop();

   	rc = i2c_lpc_ask_polling_op(ctrl);    //-- wait until write finished
   	i2c_lpc_stop();
   	return I2C_NO_ERR;
}

//----------------------------------------------------------------------------
static int m24xx_write(
    int eeprom_addr,    //-- start eeprom addr ( not included Hardware A2,A1,A0)
    char * buf,         //-- Data src buf
    int num)            //-- Bytes to write qty
{
   	int rc;
   	int b_to_wr;

   	rc = I2C_NO_ERR;
   	for(;;)
   	{

      	rc = eeprom_addr%page_size;
      	if(rc != 0) //-- not fit on page alignment
      	{
         	b_to_wr = page_size - rc;
         	if(num < b_to_wr)
            	b_to_wr = num;
         	if(b_to_wr > 0)
         	{
             	rc = i2c_lpc_m24xx_wr(eeprom_addr,buf,b_to_wr);
             	if(rc != I2C_NO_ERR)
                	break;
             	num -= b_to_wr;
             	eeprom_addr += b_to_wr;
             	buf += b_to_wr;
         	}
      	}
       	//--- write remainder(by pages,if possible)
      	while(num > 0)
      	{
          	if(num < page_size)
             	b_to_wr = num;
          	else
             	b_to_wr = page_size;

          	rc = i2c_lpc_m24xx_wr(eeprom_addr,buf,b_to_wr);
          	if(rc != I2C_NO_ERR)
             	break;
          	num -= b_to_wr;
          	eeprom_addr += b_to_wr;
          	buf += b_to_wr;
      	}

      	break;
   	}

   	return rc;
}

//----------------------------------------------------------------------------
static int m24xx_read(
  int eeprom_addr,    //-- start eeprom addr ( not included Hardware A2,A1,A0)
  char * buf,         //-- Data dst buf
  int num)            //-- Bytes to read qty
{
   	int rc;
   	int ctrl;
   	int addr_hi;
   	int addr_lo;

   	rc = I2C_NO_ERR;
   	for(;;)
   	{
      	if(num <=0)
      	{
         	rc = I2C_ERR_24XX_WRONG_NUM;
         	break;
      	}
       	//--- Here - just for addr checking
		/*
      	page_size = 0;
      	rc = m24xx_page_size(eeprom_type,eeprom_addr,&page_size);
      	if(rc != I2C_NO_ERR)
         	break;
      	if(page_size == 0)
      	{
         	rc = I2C_ERR_24XX_BAD_PAGESIZE;
         	break;
      	}
      	rc = m24xx_set_addr(eeprom_type,eeprom_addr,eeprom_cs_val,
                                           &ctrl,&addr_hi,&addr_lo);
      	if(rc != I2C_NO_ERR)
         	break;
		*/
		// *****VALE SOLO PER 24LC256 !!!!!!!!!!!!
		ctrl = (eeprom_cs<<1) & 0x07; //-- 00001110
 		ctrl |= 0xA0; //-- 1010xxxx
 		addr_hi = (eeprom_addr>>8) & 0x7F;
 		addr_lo = eeprom_addr & 0xFF;
		// **************************************

       	//--- wr START + CONTROL
      	rc = i2c_lpc_ctrl(ctrl & 0xFE); //-- Now WR (RD/WI = 0)
      	if(rc != I2C_NO_ERR)
         	break;
       	//--- wr ADDRESS
      	i2c_lpc_wr_byte(addr_hi);
      	if(addr_lo != -1)
         	i2c_lpc_wr_byte(addr_lo);

       	//--- wr START + CONTROL again - for read start
      	rc = i2c_lpc_ctrl(ctrl | 0x01); //-- Now RD (RD/WI = 1)
      	if(rc != I2C_NO_ERR)
         	break;

      	rc = i2c_lpc_rx_to_buf(buf,num);
      	if(rc != I2C_NO_ERR)
         	break;

      	i2c_lpc_stop();     //---- Set STOP ---

      	break;
   	}

   return rc;
}

/////////////////////////////////////////////////////////////////////

//*****************************************************************************
// FILENAME: 
// PARAMETERS:
// DESCRIPTION:	24LC256
// RETURNS:
//*****************************************************************************

int E2Write(int data,int address)
{
	int rc;
   	int ctrl;
   	int addr_hi;
   	int addr_lo;
	
   	ctrl = (eeprom_cs<<1) & 0x07; //-- 00001110
    ctrl |= 0xA0; //-- 1010xxxx
    addr_hi = (address>>8) & 0x7F;
    addr_lo = address & 0xFF;
   	
   	//--- wr START + CONTROL
   	rc = i2c_lpc_ctrl(ctrl); //-- Now WR (RD/WI = 0)
   	if(rc != I2C_NO_ERR)
      	return rc;
   	//--- wr ADDRESS
   	i2c_lpc_wr_byte(addr_hi);
   	if(addr_lo != -1)
      	i2c_lpc_wr_byte(addr_lo);
   	//---  Write  data
   	i2c_lpc_wr_byte(data);
   	//-----------------------
   	i2c_lpc_stop();

   	rc = i2c_lpc_ask_polling_op(ctrl);    //-- wait until write finished
   	i2c_lpc_stop();

   	return rc;
	
}

//*****************************************************************************
// FILENAME: 
// PARAMETERS:
// DESCRIPTION:	24LC256
// RETURNS:
//*****************************************************************************

int E2Read(char *data, int address)
{
   	int rc;
   	int ctrl;
   	int addr_hi;
   	int addr_lo;

   	rc = I2C_NO_ERR;
   	for(;;)
   	{
       	//--- Here - just for addr checking
		ctrl = (eeprom_cs<<1) & 0x07; //-- 00001110
        ctrl |= 0xA0; //-- 1010xxxx
        addr_hi = (address>>8) & 0x7F;
        addr_lo = address & 0xFF;
       	//--- wr START + CONTROL
      	rc = i2c_lpc_ctrl(ctrl & 0xFE); //-- Now WR (RD/WI = 0)
      	if(rc != I2C_NO_ERR)
         	break;
       	//--- wr ADDRESS
      	i2c_lpc_wr_byte(addr_hi);
      	if(addr_lo != -1)
         	i2c_lpc_wr_byte(addr_lo);

       	//--- wr START + CONTROL again - for read start
      	rc = i2c_lpc_ctrl(ctrl | 0x01); //-- Now RD (RD/WI = 1)
      	if(rc != I2C_NO_ERR)
         	break;

		I21CONCLR = I2C_FLAG_AA | I2C_FLAG_SI;  // After next will NO ASK
      	while(!(I21CONSET & I2C_FLAG_SI));  // End Data from slave;
      	*data = (char)I21DAT;

      	i2c_lpc_stop();     //---- Set STOP ---

      	break;
   	}

   	return rc;

}

//*****************************************************************************
// FILENAME: 
// PARAMETERS:
// DESCRIPTION:
// RETURNS:
//*****************************************************************************

int E2BlockWrite(char *ptr_data,int start_address,int size)
{
	return m24xx_write(start_address,ptr_data,size);
}

//*****************************************************************************
// FILENAME: 
// PARAMETERS:
// DESCRIPTION:
// RETURNS:
//*****************************************************************************

int E2BlockRead(char *ptr_data,int start_address,int size)
{
	return m24xx_read(start_address,ptr_data,size);
}

//*****************************************************************************
// FILENAME: 
// PARAMETERS:
// DESCRIPTION:
// RETURNS:
//*****************************************************************************

void open_i2c(void)
{
	//-----------------------------------------------
	// Config eeprom i2c
	//-----------------------------------------------  
	i2c_lpc_init(I2C_SPEED_400);
}
