/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#include "IO/io_emulator.h"
#include "IO/video_card.h"
#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "util/types.h"
#include "../../arch/interface.h"
#include "../../common/util/error.h"

#include <stdio.h>
#include <pthread.h>

#define FAKE_KEY_PRESS IO_SIGNAL_CUSTOM

/*
static void *run_emulator(void *arg)
{
	emulator_t *em = arg;
	int res;
	do {
		res = emulator_execute(em);
	} while(res != 1);
	return NULL;
}
*/

static void *run_emulator(void *arg)
{
	emulator_t *em = arg;

	int res;
	do {
		res = emulator_execute(em);
	} while(res != 1);
	return NULL;
}

static pthread_t start_emulator(emulator_t *em)
{
	pthread_t thread;
	pthread_create(&thread, NULL, &run_emulator, em);
	return thread;
}

int main(int argc, const char *argv[])
{
	cpu_t		*cpu;
	memory_t	*mem;
	io_t		*io;
	video_card_t	*vc;
	emulator_t	*em;
	pthread_t	emulator_thread;

	int res;

	cpu = new_cpu(MEMORY_SIZE - 8);
	mem = new_memory();
	io = new_io_emulator();
	vc = new_video_card(mem, ADDRESS_GRAPHICS_CARD_DATA, ADDRESS_GRAPHICS_CARD_ADDRESS);
	em = new_emulator(cpu, mem, io, vc);

	memory_from_file(mem, argv[1]);
	/*do {
		getchar();
		res = emulator_execute(em);
		emulator_debug(em);
	} while(res != 1);
	*/
	emulator_thread = start_emulator(em);
	do {
		res = fgetc(stdin);
		io_write_portb(io, (char)res, IO_DEVICE_ACCESS);
		io_write_porta(io, FAKE_KEY_PRESS, IO_DEVICE_ACCESS);
		io_interrupt_and_wait_until_porta_read(io);
	} while (res != 'x');
	pthread_join(emulator_thread, NULL);

}