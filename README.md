# XMODEM Library for STM32 (HAL-Based)

This repository provides a lightweight implementation of the **XMODEM file transfer protocol** for STM32 microcontrollers using the **STM32 HAL** library.  
It supports both **transmit** and **receive** operations over UART with **interrupt-driven TX** and **buffered RX** handling.

---

## ✨ Features
- ✅ 128-byte packet mode (**XMODEM-128**)  
- ✅ **CRC16** and **Checksum** support  
- ✅ UART-based **send/receive** with STM32 HAL  
- ✅ Built-in **timeout management** for TX/RX  
- ✅ Clear **error codes** for timeout, cancel, and retransmission  
- ✅ Configurable **packet size** and **retry count**  

---

## 📂 File Structure
├── XMODEM.c // Core implementation
├── XMODEM.h // Header definitions (enums, error codes, prototypes)
├── main.c // Example usage (integrate into STM32 project)

yaml
Copy code

---

## ⚙️ Configuration
Update the UART handle in **XMODEM.c** to match your project setup:

```c
#define Xmodem_Uart huart6   // Change this to your UART instance
Note: Ensure that the UART instance is initialized in interrupt mode before using this library.

🚀 API Overview
Send Data
c
Copy code
int xmodem_send(uint8_t *data, uint32_t length, uint32_t timeout_ms);
Sends a buffer using the XMODEM protocol.

Returns:

XMODEM_OK – success

XMODEM_ERR_TIMEOUT – operation timed out

XMODEM_ERR_CANCEL – transfer canceled

Receive Data
c
Copy code
int xmodem_receive(uint8_t *dest, uint32_t max_len, uint32_t timeout_ms);
Receives data via the XMODEM protocol.

Returns:

> 0 – number of bytes received

XMODEM_ERR_TIMEOUT – operation timed out

XMODEM_ERR_CANCEL – transfer canceled

Send CRC Request
c
Copy code
void xm_c_send(void);
Sends the 'C' character to request a CRC-based transfer from the sender.

📋 Example Usage
Sending
c
Copy code
uint8_t buffer[] = "Hello, XMODEM!";
if (xmodem_send(buffer, sizeof(buffer), 10000) == XMODEM_OK) {
    printf("Transfer successful!\n");
} else {
    printf("Transfer failed.\n");
}
Receiving
c
Copy code
uint8_t rx_buffer[1024];
int len = xmodem_receive(rx_buffer, sizeof(rx_buffer), 15000);
if (len > 0) {
    printf("Received %d bytes via XMODEM.\n", len);
} else {
    printf("Receive failed.\n");
}
🔧 Dependencies
STM32 HAL drivers

Configured UART instance with interrupts enabled

📌 Notes
Default packet size: 128 bytes

Retries per packet: 10

Timeout values configurable in XMODEM.c

Padding for short packets: 0x00 (modifiable)

👉 This library is intended to be portable, lightweight, and easy to integrate into STM32 projects requiring reliable file/data transfers over UART.
