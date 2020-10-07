/*
 * cbuf.h
 *
 *  Created on: 7.10.2020
 *      Author: user
 */

#ifndef H_CBUF_H_
#define H_CBUF_H_

#include "types.h"

typedef struct cbuf {
	uint8_t  		*buf;

	uint32_t		head;
	uint32_t		tail;
	uint32_t		size;	/* size of circle buffer*/
	uint32_t 		is_full;
}cbuf_t;

status_t cbuf_put(cbuf_t *cctrl, uint8_t data);
status_t cbuf_get(cbuf_t *cctrl, uint8_t *data);
status_t cbuf_init(cbuf_t *cctrl, uint8_t *buf, uint32_t size);

#endif /* H_CBUF_H_ */
