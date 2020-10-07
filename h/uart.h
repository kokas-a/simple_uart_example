/*
 * uart.h
 *
 *  Created on: 7.10.2020
 *      Author: user
 */

#ifndef H_UART_H_
#define H_UART_H_

#define MAX_NUM_OF_UARTS	(3)

struct uart_bsp_s{
	void 		*base_addr;
	uint32_t 	base_freq;
	uint32_t    reg_len;
};

struct uart_hdlr_s{
	void *p;
};

#define CMD_UART_BAUDRATE_SET (0x80000001)
#define CMD_UART_STOP_BIT_SET (0x80000002)
#define CMD_UART_PARITY_SET   (0x80000003)
#define CMD_UART_WLEN_SET     (0x80000004)

#define CMD_PARAM_PARITY_NO   (0x0)
#define CMD_PARAM_PARITY_EVEN (0x1)
#define CMD_PARAM_PARITY_ODD  (0x2)

#define CMD_PARAM_STOP_1      (0x1)
#define CMD_PARAM_STOP_2      (0x2)

#define CMD_PARAM_WLEN_5      (0x5)
#define CMD_PARAM_WLEN_6      (0x6)
#define CMD_PARAM_WLEN_7      (0x7)
#define CMD_PARAM_WLEN_8      (0x8)

struct uart_hdlr_s * uart_probe(struct uart_bsp_s *ub);
status_t uart_ctl(struct uart_hdlr_s *h, int cmd, void *arg);
status_t uart_write_c(struct uart_hdlr_s *h, uint8_t c);
status_t uart_read_c(struct uart_hdlr_s *h, uint8_t *c);

#endif /* H_UART_H_ */
