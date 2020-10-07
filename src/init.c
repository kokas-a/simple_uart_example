/*
 * init.c
 *
 *  Created on: 5.10.2020
 *      Author: user
 */
#include "types.h"
#include "uart.h"
#include "cbuf.h"

#define CBUF_SIZE	(256)

static struct uart_bsp_s uart_bsp[] = {
		{
			.base_addr = (void *)0x180003f8UL,
			.base_freq = 1843200,
			.reg_len  =  1,
		},
		{
			.base_addr = (void *)0x1f000900UL,
			.base_freq = 1843200,
			.reg_len  =  1,
		},
};

static uint8_t buf[CBUF_SIZE];

/******************************************************************************
 *
 *	Program entry
 */
void init(void)
{
	struct uart_hdlr_s *h_writer, *h_reader;
	status_t res;
	cbuf_t cbuf;
	uint8_t in, out;

	h_writer = uart_probe(&uart_bsp[0]);
	if(!h_writer){
		/* goto HANG() loop */
		return;
	}

	res  = uart_ctl(h_writer, CMD_UART_BAUDRATE_SET, (void *)115200);
	res |= uart_ctl(h_writer, CMD_UART_STOP_BIT_SET, (void *)1);
	res |= uart_ctl(h_writer, CMD_UART_WLEN_SET, (void *)CMD_PARAM_WLEN_8);
	res |= uart_ctl(h_writer, CMD_UART_PARITY_SET, \
			(void *)CMD_PARAM_PARITY_EVEN);

	if(res){
		/* goto HANG() loop */
		return;
	}

	h_reader = uart_probe(&uart_bsp[1]);
	if(!h_reader){
		/* goto HANG() loop */
		return;
	}

	res  = uart_ctl(h_reader, CMD_UART_BAUDRATE_SET, (void *)9600);
	res |= uart_ctl(h_reader, CMD_UART_STOP_BIT_SET, (void *)1);
	res |= uart_ctl(h_reader, CMD_UART_WLEN_SET, (void *)CMD_PARAM_WLEN_8);
	res |= uart_ctl(h_reader, CMD_UART_PARITY_SET, \
			(void *)CMD_PARAM_PARITY_EVEN);

	if(res){
		/* goto HANG() loop */
		return;
	}

	/* Init cbuf */
	cbuf_init(&cbuf, buf, CBUF_SIZE);

	/* Main loop */
	while(1){
		res = uart_read_c(h_reader, &in);
		if(!res){
			cbuf_put(&cbuf, in);
		}

		res = cbuf_get(&cbuf, &out);
		if(!res){
			uart_write_c(h_writer, out);
		}
	}

	return;
}
