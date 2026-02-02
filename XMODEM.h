/*

Created By Mausam Raj
Github link : https://github.com/Mausam678/XMODEM-STM32H5/blob/main/XMODEM.c
 */

#ifndef INC_XMODEM_H_
#define INC_XMODEM_H_

#include <stdint.h>
#include <stdbool.h>


#define XMODEM_OK            0
#define XMODEM_ERR_TIMEOUT  -1
#define XMODEM_ERR_ACK      -2
#define XMODEM_ERR_CANCEL   -3
#define XMODEM_ERR_UNKNOWN  -4


int xmodem_send(uint8_t *data, uint32_t length, uint32_t timeout_ms);
int xmodem_receive(uint8_t *dest, uint32_t max_len, uint32_t timeout_ms);
void xm_c_send(void);


#endif /* INC_XMODEM_H_ */

