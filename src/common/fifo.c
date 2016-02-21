///////////////////////////////////////////////////////////////////////////////
//                      fifo implementation
///////////////////////////////////////////////////////////////////////////////

#include "fifo.h"

///////////////////////////////////////////////////////////////////////////////
// fifo_init
//
// This initializes the FIFO structure with the given buffer and size
//

void fifo_init( fifo_t *f, uint8_t *buf, uint16_t size )
{
    f->head = 0;
    f->tail = 0;
    f->size = size;
    f->buf = buf;
}

///////////////////////////////////////////////////////////////////////////////
// fifo_read
//
// This reads nbytes bytes from the FIFO
// The number of bytes read is returned
//

uint16_t fifo_read( fifo_t *f, void *buf, uint16_t nbytes )
{
    uint16_t i;
    uint8_t *p;
    p = buf;
    
    for ( i=0; i < nbytes; i++){
        
        if ( f->tail != f->head ){      // see if any data is available
            *p++ = f->buf[ f->tail ];   // grab a byte from the buffer
            f->tail++;                  // increment the tail
            if( f->tail == f->size ){   // check for wrap-around
                f->tail = 0;
            }
        } 
        else {
            return i;                   // number of bytes read 
        }
     }
    
    return nbytes;
}

///////////////////////////////////////////////////////////////////////////////
// fifo_write
//
// This writes up to nbytes bytes to the FIFO
// If the head runs in to the tail, not all bytes are written
// The number of bytes written is returned
//

uint16_t fifo_write( fifo_t *f, const void *buf, uint32_t nbytes)
{
    uint32_t i;
    const uint8_t *p;
    p = buf;
 
    for ( i=0; i < nbytes; i++ ){
        
        // first check to see if there is space in the buffer
        if ( (f->head + 1 == f->tail) ||
                ( (f->head + 1 == f->size) && (f->tail == 0) ) ) {
            return i; 					// no more room
        } 
        else {
            f->buf[ f->head ] = *p++;
            f->head++;  				// increment the head
            if ( f->head == f->size ){  // check for wrap-around
                f->head = 0;
            }
        }
    }

    return nbytes;
}


///////////////////////////////////////////////////////////////////////////////
// fifo_write
//
// Get the size of the FIFO
//
// The number of bytes that can be written to the fifo is returned
//

uint16_t fifo_getFree( fifo_t *f )
{
    if ( f->head < f->tail ) {
        return (f->tail - f->head);
    }
    else {
        return (f->size + (f->tail - f->head));
    }
}


// ------------------------------------------------------------------------------

/*
static fifo_t * uart_fifo;

void uart_fifo_init(fifo_t *fifo){
    uart_fifo = fifo;
}

void uart_isr(void){
    
    uint8_t incoming_byte;
    
    // execute any required ISR entrance code

    while( UART_REGISTER_FLAG_DATA_READY ) { 			//check the register for incoming data
        incoming_byte = UART_REGISTER_READ_DATA_BYTE; 	//read the UART data
        fifo_write(uart_fifo, &incoming_byte, 1); 		//write the data to the fifo
    }

    // execute any required ISR exit code
}
*/

