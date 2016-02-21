///////////////////////////////////////////////////////////////////////////////
//                  Header for fifo implementation
///////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion -------------------------------------
#ifndef __FIFO_20131120_H
#define __FIFO_20131120_H

#ifdef __cplusplus
 extern "C" {
#endif 

typedef struct {
    uint8_t *buf;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} fifo_t;


// Prototypes
void fifo_init( fifo_t *f, uint8_t *buf, uint16_t size );
uint16_t fifo_read( fifo_t *f, void *buf, uint16_t nbytes );
uint16_t fifo_write( fifo_t *f, const void *buf, uint32_t nbytes );
uint16_t fifo_getFree( fifo_t *f );

#ifdef __cplusplus
}
#endif

#endif /* __FIFO_20131120_H */
