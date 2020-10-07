/*
 * cbuf.c
 *
 *  Created on: 7.10.2020
 *      Author: user
 */
#include "types.h"
#include "cbuf.h"

/******************************************************************************
 *
 */
status_t cbuf_put(cbuf_t *cctrl, uint8_t data)
{
	uint8_t *to;

	if(cctrl->is_full)
		return -1;

	/* Send the next message */
	to = &cctrl->buf[cctrl->head];

	*to = data;

	/* Increment the head index */
	if (++(cctrl->head) >= cctrl->size)
		cctrl->head = 0;

	if (cctrl->head == cctrl->tail)
		cctrl->is_full = 1;

	return 0;
}

/******************************************************************************
 *
 */
status_t cbuf_get(cbuf_t *cctrl, uint8_t *data)
{
	uint8_t *from;

	if((cctrl->tail == cctrl->head) && !cctrl->is_full){
		return (-1);
	}

	from = &cctrl->buf[cctrl->tail];

	*data = *from;

	if(++(cctrl->tail) >= cctrl->size)
		cctrl->tail = 0;

	cctrl->is_full = 0;

	return 0;
}

/******************************************************************************
 *
 */
status_t cbuf_init(cbuf_t *cctrl, uint8_t *buf, uint32_t size)
{
	cctrl->buf = buf;
	cctrl->size = size;

	return 0;
}

