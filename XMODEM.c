/*
Created by Mausam Raj 

Github link : https://github.com/Mausam678/XMODEM-STM32H5/blob/main/XMODEM.c


*/



#include "XMODEM.h"
//#include "stm32h5xx_hal.h"  // or your platform header
#include "main.h"
#include <string.h>

// Must be defined elsewhere
//extern UART_HandleTypeDef huart6;
#define Xmodem_Uart huart6


// XMODEM control bytes
#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define SUB 0x1A

#define SEQ_START 1
#define PACKET_SIZE 128
#define MAX_RETRIES 10
#define RX_BYTE_TIMEOUT_MS 5000  // waiting for initial NAK/CRC request
#define PACKET_ACK_TIMEOUT_MS 1000

//uint8_t rx_byte;
_Bool rx_received=0;
_Bool tx_done=0;

uint8_t XM_BUF[XM_BUF_S];
uint8_t rx_byte  = 0;
uint16_t rx_head = 0;
uint16_t rx_tail = 0;


static uint16_t crc16_xmodem(const uint8_t *buf, uint32_t len)
{
    uint16_t crc = 0;
    for (uint32_t i = 0; i < len; ++i) {
        crc ^= ((uint16_t)buf[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}



// Wait for one rx byte with timeout, returns -1 on timeout, or the byte
// Helper to transmit buffer via DMA and wait for completion with timeout
static int tx_dma_wait(const uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    tx_done = false;
    if (HAL_UART_Transmit_IT(&Xmodem_Uart, (uint8_t *)buf, len) != HAL_OK) {
        return -1;
    }
    uint32_t start = HAL_GetTick();
    while (!tx_done) {
        if ((HAL_GetTick() - start) > timeout_ms) return -1;
    }
    return 0;
}



static int wait_for_rx(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (rx_head == rx_tail) {
        if ((HAL_GetTick() - start) > timeout_ms) return -1;
    }
    uint8_t val = XM_BUF[rx_tail];
    rx_tail = (rx_tail + 1) % XM_BUF_S;
    return val;
}


// Main send routine

int xmodem_send(uint8_t *data, uint32_t length, uint32_t timeout_ms)
{

    rx_received = false;

//    if (HAL_UART_Receive_IT(&Xmodem_Uart, (uint8_t *)&rx_byte, 1) != HAL_OK)
//    {
//    	printf("Interrupt receive failed\n");
//    }

    uint32_t start_wait = HAL_GetTick();
    int ch = -1;
    while (1)
    {
        ch = wait_for_rx(RX_BYTE_TIMEOUT_MS);
        if (ch == -1)
        {
              return XMODEM_ERR_TIMEOUT;
        }
        if (ch == 'C' || ch == NAK) break;
        if (ch == CAN) return XMODEM_ERR_CANCEL;
        if ((HAL_GetTick() - start_wait) > timeout_ms) return XMODEM_ERR_TIMEOUT;
    }

    bool use_crc = (ch == 'C');
    uint8_t packet[PACKET_SIZE];
    uint32_t bytes_sent = 0;
    uint8_t block_num = SEQ_START;
    uint8_t retries;
    while (bytes_sent < length)
    {

        uint32_t remaining = length - bytes_sent;
        printf("Remaing %ld\n",remaining);
        uint32_t copy_len = (remaining >= PACKET_SIZE) ? PACKET_SIZE : remaining;
        memset(packet, 0x00, PACKET_SIZE);
        memcpy(packet, &data[bytes_sent], copy_len);
        if (copy_len < PACKET_SIZE) {
            // pad with SUB (0x1A) as usual
            for (uint32_t i = copy_len; i < PACKET_SIZE; ++i) packet[i] = 0x00;
        }

        // build frame: SOH, blk, ~blk, payload, crc(2)/checksum(1)
        uint8_t frame[3 + PACKET_SIZE + 2];
        uint32_t idx = 0;
        frame[idx++] = SOH;
        frame[idx++] = block_num;
        frame[idx++] = 255 - block_num;
        memcpy(&frame[idx], packet, PACKET_SIZE);
        idx += PACKET_SIZE;
        if (use_crc)
        {
            uint16_t crc = crc16_xmodem(packet, PACKET_SIZE);
            frame[idx++] = (uint8_t)((crc >> 8) & 0xFF);
            frame[idx++] = (uint8_t)(crc & 0xFF);
        }
        else
        {
            uint8_t chks = 0;
            for (uint32_t i = 0; i < PACKET_SIZE; ++i) chks += packet[i];
            frame[idx++] = chks;
        }
        retries = 0;
        bool packet_sent = false;
        while (retries < MAX_RETRIES)
        {
            if (tx_dma_wait(frame, idx, PACKET_ACK_TIMEOUT_MS) != 0)
            {
                retries++;
                continue;
            }
            // wait for ACK/NAK/CAN
            int r = wait_for_rx(PACKET_ACK_TIMEOUT_MS);
            if (r == -1)
            {
                retries++;
                continue;
            }
            if (r == ACK)
            {
                packet_sent = true;
                break;
            }
            else if (r == NAK)
            {
                retries++;
                continue;
            }
            else if (r == CAN)
            {
                return XMODEM_ERR_CANCEL;
            }
            else
            {
                retries++;
                continue;
            }
        }

        if (!packet_sent) return XMODEM_ERR_TIMEOUT;

        bytes_sent += PACKET_SIZE;
        block_num++;
    }

    // send EOT, wait for ACK
    retries = 0;
    while (retries < MAX_RETRIES)
    {
        uint8_t eot = EOT;
        if (tx_dma_wait(&eot, 1, PACKET_ACK_TIMEOUT_MS) != 0)
        {
            retries++;
            continue;
        }
        int r = wait_for_rx(PACKET_ACK_TIMEOUT_MS);
        if (r == ACK) return XMODEM_OK;
        else if (r == NAK)
        {
            retries++;
            continue;
        }
        else if (r == CAN)
        {
            return XMODEM_ERR_CANCEL;
        }
        else
        {
            retries++;
        }
    }

    return XMODEM_ERR_TIMEOUT;
}

void xm_c_send(void)
{
	uint8_t req = 'C';
	HAL_UART_Transmit(&Xmodem_Uart, &req, 1, 1);
//	HAL_UART_Receive_IT(&Xmodem_Uart, (uint8_t *)&rx_byte, 1);

}
int xmodem_receive(uint8_t *dest, uint32_t max_len, uint32_t timeout_ms)
{
    uint8_t packet[PACKET_SIZE];
    uint8_t block_num_expected = 1;
    uint32_t bytes_received = 0;

//    uint8_t req = 'C';
//    HAL_UART_Transmit(&Xmodem_Uart, &req, 1, HAL_MAX_DELAY);
//    HAL_UART_Receive_IT(&Xmodem_Uart, (uint8_t *)&rx_byte, 1);

    while (1)
    {
        int ch = wait_for_rx(timeout_ms);
        if (ch == -1) return XMODEM_ERR_TIMEOUT;
        if (ch == SOH)
        {
            int blk = wait_for_rx(timeout_ms);
            int blk_inv = wait_for_rx(timeout_ms);
            if (blk < 0 || blk_inv < 0) return XMODEM_ERR_TIMEOUT;
            if ((blk + blk_inv) != 0xFF || blk != block_num_expected)
            {
                uint8_t nak = NAK;
                HAL_UART_Transmit(&Xmodem_Uart, &nak, 1, 100);
                continue;
            }
            for (int i = 0; i < PACKET_SIZE; i++)
            {
                int d = wait_for_rx(timeout_ms);
                if (d < 0) return XMODEM_ERR_TIMEOUT;
                packet[i] = (uint8_t)d;
            }
            int crc_hi = wait_for_rx(timeout_ms);
            int crc_lo = wait_for_rx(timeout_ms);
            if (crc_hi < 0 || crc_lo < 0) return XMODEM_ERR_TIMEOUT;
            uint16_t crc_recv = ((uint16_t)crc_hi << 8) | (uint16_t)crc_lo;
            uint16_t crc_calc = crc16_xmodem(packet, PACKET_SIZE);
            if (crc_recv != crc_calc)
            {
                uint8_t nak = NAK;
                HAL_UART_Transmit(&Xmodem_Uart, &nak, 1, 100);
                continue;
            }
            uint32_t copy_len = (bytes_received + PACKET_SIZE <= max_len) ? PACKET_SIZE : (max_len - bytes_received);
            memcpy(&dest[bytes_received], packet, copy_len);
            bytes_received += copy_len;
            uint8_t ack = ACK;
            HAL_UART_Transmit(&Xmodem_Uart, &ack, 1, 100);
            block_num_expected++;
        }
        else if (ch == EOT)
        {
            uint8_t ack = ACK;
            HAL_UART_Transmit(&Xmodem_Uart, &ack, 1, 100);
            return bytes_received; // transfer complete
        }
        else if (ch == CAN)
        {
            return XMODEM_ERR_CANCEL;
        }
    }
}

