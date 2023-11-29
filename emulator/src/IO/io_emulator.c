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
static void	wait_for_interrupt_clear(io_t *io);

io_t	*new_io_emulator()
{
	io_t	*res;

	res = calloc(1, sizeof(io_t));
	pthread_mutex_init(&res->mutex, NULL);
	pthread_cond_init(&res->irq_cond, NULL);
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

	DO_LOCKED(&io->mutex, res = io->porta.port);
	return res;
}

u16_t	io_read_portb(io_t *io, io_access_type_t caller)
{
	u16_t res;

	DO_LOCKED(&io->mutex, res = io->portb.port);
	return res;
}

static void wait_for_interrupt_clear(io_t *io)
{
	while (io->irq)
		pthread_cond_wait(&io->irq_cond, &io->mutex);
}

void	io_interrupt_and_wait(io_t *io)
{
	if (!io->interrupt_enabled)
		return;

	DO_LOCKED(&io->mutex, {
		io->irq = true;
		wait_for_interrupt_clear(io);
	})
}

void	io_interrupt(io_t *io)
{
	if (!io->interrupt_enabled)
		return;
	DO_LOCKED(&io->mutex,
		io->irq = true;
	);
}

void	io_wait_and_interrupt(io_t *io)
{
	if (!io->interrupt_enabled)
		return;

	DO_LOCKED(&io->mutex, {
		if (!io->irq) {
			io->irq = true;
		} else {
			wait_for_interrupt_clear(io);
			io->irq = true;
		}
	});
}

void	io_clear_interrupt(io_t *io)
{
	io->irq = false;
	pthread_cond_broadcast(&io->irq_cond);
}
