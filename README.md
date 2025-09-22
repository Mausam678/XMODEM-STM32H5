XMODEM Library for STM32 (HAL-Based)

This repository provides an implementation of the XMODEM protocol for STM32 microcontrollers using the STM32 HAL library.
It supports both send and receive operations over UART using interrupt-driven transmit and buffered receive handling.

✨ Features

✅ XMODEM 128-byte packet mode

✅ CRC16 and Checksum support

✅ UART-based send/receive with HAL

✅ Timeout handling for RX/TX

✅ Error codes for timeout, cancel, and retransmission

✅ Configurable packet size and retries

📂 File Structure
├── XMODEM.c   # Core implementation
├── XMODEM.h   # Header definitions (enums, error codes, prototypes)
├── main.c     # Example usage (integrate into STM32 project)

⚙️ Configuration

Modify the UART handle in XMODEM.c to match your project:

#define Xmodem_Uart huart6   // Change this to your UART instance


Ensure you have initialized UART in interrupt mode before using this library.

🚀 API Overview
Send Data
int xmodem_send(uint8_t *data, uint32_t length, uint32_t timeout_ms);


Sends data using the XMODEM protocol.

Returns:

XMODEM_OK on success

XMODEM_ERR_TIMEOUT on timeout

XMODEM_ERR_CANCEL if canceled

Receive Data
int xmodem_receive(uint8_t *dest, uint32_t max_len, uint32_t timeout_ms);


Receives data using the XMODEM protocol.

Returns:

Number of bytes received

XMODEM_ERR_TIMEOUT on timeout

XMODEM_ERR_CANCEL if canceled

Send CRC Request
void xm_c_send(void);


Sends 'C' to request a CRC-based transfer from the sender.

📋 Example Usage
Send Example
uint8_t buffer[] = "Hello, XMODEM!";
if (xmodem_send(buffer, sizeof(buffer), 10000) == XMODEM_OK) {
    printf("Transfer successful!\n");
} else {
    printf("Transfer failed.\n");
}

Receive Example
uint8_t rx_buffer[1024];
int len = xmodem_receive(rx_buffer, sizeof(rx_buffer), 15000);
if (len > 0) {
    printf("Received %d bytes via XMODEM.\n", len);
} else {
    printf("Receive failed.\n");
}

🔧 Dependencies

STM32 HAL drivers

A UART instance configured with interrupts

📌 Notes

Default packet size: 128 bytes

Retries per packet: 10

Timeout values are configurable in XMODEM.c

Padding for short packets is done with 0x00 (can be adjusted)
