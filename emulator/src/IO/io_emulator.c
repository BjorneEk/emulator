/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * io emulator
 *==========================================================*/

#include "io_emulator.h"
#include "../../../common/util/bit.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define BIT_IS(_v, _b, _st) (((_v) & (1 << (_b))) == (_st))
#define DO_LOCKED(_lock, _stmt) do {		\
	pthread_mutex_lock((_lock));		\
	_stmt;					\
	pthread_mutex_unlock((_lock));		\
	} while(0);
#define MONITOR(_lock, _fn) {			\
	pthread_mutex_lock((_lock));		\
	_fn;					\
	pthread_mutex_unlock((_lock));		\
	}

static void	port_write(port_t *p, u16_t val, io_access_type_t caller);
static u16_t	read_porta(io_t *io, io_access_type_t caller);
static u16_t	read_portb(io_t *io, io_access_type_t caller);
static u16_t	read_porta_and_clear_interrupt(io_t *io, io_access_type_t caller);
static u16_t	read_portb_and_clear_interrupt(io_t *io, io_access_type_t caller);
static void	wait_for_interrupt_clear(io_t *io);

io_t	*new_io_emulator()
{
	io_t	*res;

	res = calloc(1, sizeof(io_t));
	pthread_mutex_init(&res->mutex, NULL);
	pthread_cond_init(&res->irq_cond, NULL);
	res->read_porta = read_porta;
	res->read_portb = read_portb;
	res->interrupt_enabled = false;
	return res;
}

static void port_write(port_t *p, u16_t val, io_access_type_t caller)
{
	int		i;
	u8_t		bit;

	for (i = 0; i < 16; i++) {
		if (caller == IO_INTERNAL_ACCESS)
			p->port = (p->ddr & val) | (~p->ddr & p->port);
		else
			p->port = (~p->ddr & val) | (p->ddr & p->port);
	}
}

static u16_t	read_porta(io_t *io, io_access_type_t caller)
{
	return io->porta.port;
}

static u16_t	read_portb(io_t *io, io_access_type_t caller)
{
	return io->portb.port;
}

static u16_t	read_porta_and_clear_interrupt(io_t *io, io_access_type_t caller)
{
	u16_t res;

	res = read_porta(io, caller);
	if (caller == IO_INTERNAL_ACCESS) {
		io_clear_interrupt(io);
		io->read_porta = read_porta;
	}
	return res;
}

static u16_t	read_portb_and_clear_interrupt(io_t *io, io_access_type_t caller)
{
	u16_t res;

	res = read_portb(io, caller);
	if (caller == IO_INTERNAL_ACCESS) {
		io_clear_interrupt(io);
		io->read_portb = read_portb;
	}
	return res;
}

void	io_write_porta(io_t *io, u16_t data, io_access_type_t caller) MONITOR(&io->mutex,
{

	if (caller == IO_INTERNAL_ACCESS) {
		switch(msb_16(data)) {
			case IO_SIGNAL_ENABLE_INTERRUPT:
				io->interrupt_enabled = true;
				break;
			case IO_SIGNAL_DISSABLE_INTERRUPT:
				io->interrupt_enabled = false;
			case IO_SIGNAL_CLEAR_INTERRUPT:
				io_clear_interrupt(io);
				break;
			default:
				break;
		}
	}
	port_write(&io->porta, data, caller);
})

void	io_write_portb(io_t *io, u16_t data, io_access_type_t caller) MONITOR(&io->mutex,
{
	port_write(&io->portb, data, caller);
})

void	io_write_ddra(io_t *io, u16_t data) MONITOR(&io->mutex,
{
	io->porta.ddr = data;
})

void	io_write_ddrb(io_t *io, u16_t data) MONITOR(&io->mutex,
{
	io->portb.ddr = data;
})

u16_t	io_read_porta(io_t *io, io_access_type_t caller)
{
	u16_t res;

	DO_LOCKED(&io->mutex, res = io->read_porta(io, caller));
	return res;
}

u16_t	io_read_portb(io_t *io, io_access_type_t caller)
{
	u16_t res;

	DO_LOCKED(&io->mutex, res = io->read_portb(io, caller));
	return res;
}

static void wait_for_interrupt_clear(io_t *io)
{
	while (io->irq)
		pthread_cond_wait(&io->irq_cond, &io->mutex);
}

static void	_interrupt_and_wait(io_t *io, read_port_func_t *fn, read_port_func_t fn_)
{
	if (!io->interrupt_enabled)
		return;

	pthread_mutex_lock(&io->mutex);

	wait_for_interrupt_clear(io);	/* make shure an interrupt is not ongoing */
	if (fn != NULL)
		*fn = fn_;
	io->irq = true;			/* create interrupt not ongoing */
	wait_for_interrupt_clear(io);
	pthread_mutex_unlock(&io->mutex);
}

void	io_interrupt_and_wait(io_t *io)
{
	_interrupt_and_wait(io, NULL, NULL);
}

void	io_interrupt_and_wait_until_porta_read(io_t *io)
{

	_interrupt_and_wait(io, &io->read_porta, read_porta_and_clear_interrupt);
}

void	io_interrupt_and_wait_until_portb_read(io_t *io)
{
	_interrupt_and_wait(io, &io->read_portb, read_portb_and_clear_interrupt);
}

void	io_clear_interrupt(io_t *io)
{
	io->irq = false;
	pthread_cond_broadcast(&io->irq_cond);
}
