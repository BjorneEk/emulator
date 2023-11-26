/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * io emulator
 *==========================================================*/
#ifndef _IO_H_
#define _IO_H_

#include "../../../common/util/types.h"
#include <pthread.h>

typedef struct io_emulator io_t;

typedef enum io_ddr_bit_state {
	IO_INPUT	= 0,
	IO_OUTPUT	= 1
} ddr_bit_t;

typedef enum io_access_type {
	IO_DEVICE_ACCESS	= 0,
	IO_INTERNAL_ACCESS	= 1
} io_access_type_t;

typedef enum io_signal {
	IO_SIGNAL_ENABLE_INTERRUPT	= 0,
	IO_SIGNAL_DISSABLE_INTERRUPT	= 1,
	IO_SIGNAL_CLEAR_INTERRUPT	= 2,
	IO_SIGNAL_CUSTOM		= 3
} io_signal_t;

typedef struct io_port {
	u16_t		port;
	u16_t		ddr;
} port_t;

typedef u16_t (*read_port_func_t)(io_t *io, io_access_type_t caller);

typedef struct io_emulator {
	port_t		porta;
	port_t		portb;

	pthread_mutex_t	mutex;
	pthread_cond_t	irq_cond;
	bool		irq;
	bool		interrupt_enabled;

	read_port_func_t	read_porta;
	read_port_func_t	read_portb;
} io_t;

io_t	*new_io_emulator();

void	io_write_ddra(io_t *io, u16_t data);
void	io_write_ddrb(io_t *io, u16_t data);
void	io_write_porta(io_t *io, u16_t data, io_access_type_t caller);
void	io_write_portb(io_t *io, u16_t data, io_access_type_t caller);

u16_t	io_read_porta(io_t *io, io_access_type_t caller);
u16_t	io_read_portb(io_t *io, io_access_type_t caller);

static inline u16_t	io_read_ddra(io_t *io)
{
	return io->porta.ddr;
}

static inline u16_t	io_read_ddrb(io_t *io)
{
	return io->portb.ddr;
}

void	io_interrupt_and_wait(io_t *io);
void	io_interrupt_and_wait_until_porta_read(io_t *io);
void	io_interrupt_and_wait_until_portb_read(io_t *io);
void	io_clear_interrupt(io_t *io);

static inline bool io_irq(io_t *io)
{
	return io->irq;
}

#endif /* _IO_H_ */