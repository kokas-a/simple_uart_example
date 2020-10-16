/*
 * uart.c
 *
 *  Created on: 7.10.2020
 *      Author: user
 */
#include "types.h"
#include "uart.h"

struct uart_priv_s{
	void 		*base;
	uint32_t	base_freq;
	uint32_t    reg_len;
	uint32_t    br;
	uint32_t    parity;
	uint32_t    wlen;
	uint32_t    stop;
};

static struct uart_priv_s uart_p[MAX_NUM_OF_UARTS];
static struct uart_hdlr_s uart_h[MAX_NUM_OF_UARTS];

#define UART_RBR_INCR          0 /* (DLAB =0) Receiver Buffer Register */
#define UART_THR_INCR          0 /* (DLAB =0) Transmit Holding Register */
#define UART_DLL_INCR          0 /* (DLAB =1) Divisor Latch LSB */
#define UART_DLM_INCR          1 /* (DLAB =1) Divisor Latch MSB */
#define UART_IER_INCR          1 /* (DLAB =0) Interrupt Enable Register */
#define UART_IIR_INCR          2 /* Interrupt ID Register */
#define UART_FCR_INCR          2 /* FIFO Control Register */
#define UART_LCR_INCR          3 /* Line Control Register */
#define UART_MCR_INCR          4 /* Modem Control Register */
#define UART_LSR_INCR          5 /* Line Status Register */
#define UART_MSR_INCR          6 /* Modem Status Register */
#define UART_SCR_INCR          7 /* Scratch Pad Register */

/* LCR */
#define UART_LCR_WLS_SHIFT           (0)
#define UART_LCR_WLS_MASK            (3 << UART_LCR_WLS_SHIFT)
#  define UART_LCR_WLS_5BIT          (0 << UART_LCR_WLS_SHIFT)
#  define UART_LCR_WLS_6BIT          (1 << UART_LCR_WLS_SHIFT)
#  define UART_LCR_WLS_7BIT          (2 << UART_LCR_WLS_SHIFT)
#  define UART_LCR_WLS_8BIT          (3 << UART_LCR_WLS_SHIFT)
#define UART_LCR_STB                 (1 << 2)
#define UART_LCR_PEN                 (1 << 3)
#define UART_LCR_EPS                 (1 << 4)
#define UART_LCR_STICKY              (1 << 5)
#define UART_LCR_BRK                 (1 << 6)
#define UART_LCR_DLAB                (1 << 7)

/* FCR */
#define UART_FCR_FIFOEN				 (1 << 0)
#define UART_FCR_RXRST				 (1 << 1)
#define UART_FCR_TXRST               (1 << 2)

#define UART_LSR_THRE				 (1 << 5)
#define UART_LSR_DR				     (1 << 0)

/******************************************************************************
 *
 *	Return first unused uart_priv_s structure form static pool
 */
static struct uart_priv_s * uart_alloc_priv(void)
{
	struct uart_priv_s *p = uart_p;
	uint32_t i;

	for(i = 0; i < MAX_NUM_OF_UARTS; i++, p++){
		if(!p->base)
			return p;
	}

	return NULL;
}

/******************************************************************************
 *
 * Return first unused uart_hdlr_s structure form static pool
 */
static struct uart_hdlr_s * uart_alloc_hdlr(void)
{
	struct uart_hdlr_s *h = uart_h;
	uint32_t i;

	for(i = 0; i < MAX_NUM_OF_UARTS; i++, h++){
		if(!h->p)
			return h;
	}

	return NULL;
}

/******************************************************************************
 *
 *	Write value to HW register
 */
static void uart_write_reg(struct uart_priv_s *p, uint32_t offs,
                              uint32_t val)
{
    void *addr;

    if ((!p) || (!p->base))
        return;

    if (p->reg_len == 4){
        addr = (uint32_t *) (p->base + offs * 4);
        *(uint32_t *) addr = val;
    }
    else /* reg_len == 1 */{
        addr = (uint8_t *) (p->base + offs);
        *(uint8_t *) addr = (val & 0xff);
    }
}

/******************************************************************************
 *
 *	Get value from HW register
 */
static uint32_t uart_read_reg(struct uart_priv_s *p, uint32_t offs)
{
    void *addr;
    uint32_t tmp;

    if ((!p) || (!p->base))
        return -1;

    if (p->reg_len == 4){
        addr = (uint32_t *) (p->base + offs * 4);
        tmp = *(uint32_t *) addr;
    }
    else /* reg_len == 1 */{
        addr = (uint8_t *) (p->base + offs);
        tmp = *(uint8_t *) addr;
        tmp &= 0xff;
    }

    return tmp;
}

/******************************************************************************
 *
 *	Calculate divisor
 *
 * 	new_baud = base_freq / (16 * div) =>
 * 	div  = base_freq / new_baud / 16
 *
 * 	Skip fractional divisors
 */
static uint16_t uart_divisor(struct uart_priv_s *p, uint32_t new_baud)
{
	return (p->base_freq + (new_baud << 3)) / (new_baud << 4);
}

/******************************************************************************
 *
 *	Set baud rate for given uart
 */
static status_t uart_br_set(struct uart_priv_s *p, uint32_t br)
{
	uint16_t div;
	uint32_t lcr;

	if(!p || !br)
		return (-1);

	lcr = uart_read_reg(p, UART_LCR_INCR);

	/* Enter DLAB=1 */
	uart_write_reg(p, UART_LCR_INCR, (lcr | UART_LCR_DLAB));

	/* Set the BAUD divisor */
	div = uart_divisor(p, br);
	uart_write_reg(p, UART_DLM_INCR, div >> 8);
	uart_write_reg(p, UART_DLL_INCR, div & 0xff);

	/* Clear DLAB */
	uart_write_reg(p, UART_LCR_INCR, lcr);

	p->br = br;

	return 0;
}

/******************************************************************************
 *
 *	Set stop bits for given uart
 */
static status_t uart_stop_set(struct uart_priv_s *p, uint32_t stop)
{
	uint32_t lcr;

	if(!p)
		return (-1);

	lcr = uart_read_reg(p, UART_LCR_INCR);

	lcr &= ~UART_LCR_STB;

	if(CMD_PARAM_STOP_2 == stop){
		lcr |= UART_LCR_STB;
	}

	uart_write_reg(p, UART_LCR_INCR, lcr);

	p->stop = stop;

	return 0;
}

/******************************************************************************
 *
 *	Set word length for given uart
 */
static status_t uart_wlen_set(struct uart_priv_s *p, uint32_t wlen)
{
	uint32_t lcr;

	if(!p)
		return (-1);

	lcr = uart_read_reg(p, UART_LCR_INCR);

	lcr &= ~UART_LCR_WLS_MASK;

	switch (wlen) {
	case CMD_PARAM_WLEN_5:
		lcr |= UART_LCR_WLS_5BIT;
		break;

	case CMD_PARAM_WLEN_6:
		lcr |= UART_LCR_WLS_6BIT;
		break;

	case CMD_PARAM_WLEN_7:
		lcr |= UART_LCR_WLS_7BIT;
		break;

	default:
	case CMD_PARAM_WLEN_8:
		lcr |= UART_LCR_WLS_8BIT;
		break;
	}

	uart_write_reg(p, UART_LCR_INCR, lcr);

	p->wlen = wlen;

	return 0;
}

/******************************************************************************
 *
 *	Set parity for given uart
 */
static status_t uart_par_set(struct uart_priv_s *p, uint32_t par)
{
	uint32_t lcr;
	status_t ret_val = (-1);

	if(!p)
		return ret_val;

	lcr = uart_read_reg(p, UART_LCR_INCR);

	if(CMD_PARAM_PARITY_NO == par){
		lcr &= ~(UART_LCR_PEN | UART_LCR_EPS);
		ret_val = 0;
	}else if(CMD_PARAM_PARITY_ODD == par){
		lcr &= ~(UART_LCR_PEN | UART_LCR_EPS);
		lcr |= UART_LCR_PEN;
		ret_val = 0;
	}else if(CMD_PARAM_PARITY_EVEN == par){
		lcr |= (UART_LCR_PEN | UART_LCR_EPS);
		ret_val = 0;
	}else{
		return ret_val;
	}

	uart_write_reg(p, UART_LCR_INCR, lcr);

	p->wlen = par;

	return ret_val;
}

/******************************************************************************
 *
 */
status_t uart_write_c(struct uart_hdlr_s *h, uint8_t c)
{
	struct uart_priv_s *p;
	status_t ret_val = (-1);

	if(!h || !h->p){
		/* EINVAL */
		return ret_val;
	}

	p = (struct uart_priv_s *)h->p;

	/* Wait until UART ready for the next character. */
	while ((uart_read_reg(p, UART_LSR_INCR) & UART_LSR_THRE) == 0)
		 ;

	/* Add character to the buffer. */
	uart_write_reg(p, UART_THR_INCR, c);

	return 0;
}

/******************************************************************************
 *
 */
status_t uart_read_c(struct uart_hdlr_s *h, uint8_t *c)
{
	struct uart_priv_s *p;
	status_t ret_val = (-1);
	uint32_t reg_val;

	if(!h || !h->p){
		/* EINVAL */
		return ret_val;
	}

	p = (struct uart_priv_s *)h->p;

	if(uart_read_reg(p, UART_LSR_INCR) & UART_LSR_DR){
		/* Data is ready. A complete incoming character
		 *  has been received and transferred into the receiver
		 *  buffer register (RBR) */

		reg_val = uart_read_reg(p, UART_RBR_INCR);
		*c  = reg_val & 0xFF;

		ret_val = 0;
	}

	return ret_val;
}

/******************************************************************************
 *
 */
status_t uart_ctl(struct uart_hdlr_s *h, int cmd, void *arg)
{
	struct uart_priv_s *p;
	status_t ret_val = (-1);

	if(!h || !h->p || !arg){
		/* EINVAL */
		return ret_val;
	}

	p = (struct uart_priv_s *)h->p;

	switch(cmd){
		case CMD_UART_BAUDRATE_SET:{
			ret_val = uart_br_set(p, (uint32_t)arg);
		}break;
		case CMD_UART_STOP_BIT_SET:{
			ret_val = uart_stop_set(p, (uint32_t)arg);
		}break;
		case CMD_UART_WLEN_SET:{
			ret_val = uart_wlen_set(p, (uint32_t)arg);
		}break;
		case CMD_UART_PARITY_SET:{
			ret_val = uart_par_set(p, (uint32_t)arg);
		}break;
		default:{
			;
		}break;
	}

	return ret_val;
}

/******************************************************************************
 *
 *	uart probe e& early initialization routine
 */
struct uart_hdlr_s * uart_probe(struct uart_bsp_s *ub)
{
	struct uart_priv_s *p = NULL;
	struct uart_hdlr_s *h = NULL;
	status_t res;

	if(!ub || !ub->base_addr)
		return NULL;

	p = uart_alloc_priv();
	if(!p)
		return NULL;

	p->base = ub->base_addr;
	p->base_freq = ub->base_freq;
	p->reg_len = ub->reg_len;

	/* Reset & disable fifo */
	uart_write_reg(p, UART_FCR_INCR, (UART_FCR_RXRST | UART_FCR_TXRST));

	/* Apply default settings */
	res  = uart_br_set(p, 115200);
	res |=uart_wlen_set(p, CMD_PARAM_WLEN_8);
	res |=uart_stop_set(p, CMD_PARAM_STOP_1);
	res |=uart_par_set(p, CMD_PARAM_PARITY_NO);

	if(res){
		/* TODO free(p) */
		return NULL;
	}

	h = uart_alloc_hdlr();
	if(!h){
		/* TODO free(p) */
		return NULL;
	}

	h->p = p;

	return h;
}

