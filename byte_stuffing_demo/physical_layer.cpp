#include <unistd.h>
#include "stdlib.h"

#include "physical_layer.h"
#include "timeval_operators.h"

using namespace std;


// Corrupt --------------------------------------------------------
Corrupt::Corrupt()
{
}

Corrupt::Corrupt(double probabilities[],unsigned int length)
{
	// check for exceptions
	if (length > MAXIMUM_CORRUPT_LENGTH) {
		throw Physical_layer_exception();
	}
	for (unsigned int i = 0; i < length; i++) {
		if (!(0.0 <= probabilities[i] && probabilities[i] <= 1.0)) {
			throw Physical_layer_exception();
		}
	}

	// copy corrupt parameters; initialize corrupt_index
	this->length = length;
	if (length > 0) {
		for (unsigned int i = 0; i < length; i++ ) {
			this->probabilities[i]= probabilities[i];
		}
		index = 0;
	}

	// set random number generation
	rand_state = 1;
}

unsigned char Corrupt::corrupt_byte(unsigned char b)
{
	// no corruption from empty corrupt array
	if (length == 0) {
		return b;
	}

	// get random int, scale to [0..1], compare to corrupt element
	int r0 = rand_r(&rand_state); // get next random int
	float r1 = (double)r0 / (double)RAND_MAX; // scale to [0,1]
	if (r1 <= probabilities[index]) {
		// cout << "corrupted. old: " << (unsigned int) b;
		b ^= 0x01; // corrupt: invert low-order bit
		// cout << " new: " << (unsigned int) b << endl;
	}
	index = (index+1) % length;

	return b;
}

// Overflow_queue  --------------------------------------------

Overflow_queue::Overflow_queue()
{
	size = 0;
}

void Overflow_queue::add_last(unsigned char c, struct timeval release_time)
{
	// empty queue: add at 0
	if (size == 0) {
		first = 0;
		last = 0;
		queue[first].c = c;
		queue[first].release_time = release_time;
		size++;
	} else {
		// non-empty, non-full queue: add after last
		if (size < MAXIMUM_QUEUE_LENGTH) {
			last = (last+1) % MAXIMUM_QUEUE_LENGTH;
			queue[last].c = c;
			queue[last].release_time = release_time;
			size++;
		}
/*CHANGE: drop byte if queue full
		// full queue: overwrite first; adjust first, last
		} else {
			queue[first].c = c;
			queue[first].release_time = release_time;
			first = (first+1) % MAXIMUM_QUEUE_LENGTH;
			last = (last+1) % MAXIMUM_QUEUE_LENGTH;
		}
*/
	}
}

// pre: size > 0
void Overflow_queue::remove_first()
{
	first = (first+1) % MAXIMUM_QUEUE_LENGTH;
	size--;
}

unsigned int Overflow_queue::get_size()
{
	return size;
}

// pre: size > 0
unsigned char Overflow_queue::get_first_byte()
{
	return queue[first].c;
}

// pre: size > 0
struct timeval Overflow_queue::get_first_release_time()
{
	return queue[first].release_time;
}

// pre: size > 0
unsigned char Overflow_queue::get_last_byte()
{
	return queue[last].c;
}

// pre: size > 0
struct timeval Overflow_queue::get_last_release_time()
{
	return queue[last].release_time;
}

void Overflow_queue::print()
{
	cout << "size: " << size << " first: " << first <<
	 " last: " << last << endl;
	for (int i = 0; i < MAXIMUM_QUEUE_LENGTH; i++) {
		cout << '\t' << i << " " << (int)queue[i].c <<
			" " << queue[i].release_time.tv_sec <<
			" " << queue[i].release_time.tv_usec << endl;
	}
}

// Physical_layer_interface  --------------------------------------------

Physical_layer_interface::Physical_layer_interface(
 Physical_layer_emulator *physical_layer_emulator_p,
 unsigned int bit_rate, Corrupt &corrupt)
{
	this->physical_layer_emulator_p = physical_layer_emulator_p;
	
	// set logging flags
	send_log = 0;
	receive_log = 0;

	// compute delay per byte and convert to timeval
	int us_per_byte = (8 * 1000000) / bit_rate;
	byte_delay.tv_sec = us_per_byte / 1000000;
	byte_delay.tv_usec = us_per_byte % 1000000;

	this->corrupt = corrupt;
}

char Physical_layer_interface::get_a_b_string()
{
	return 'a';
}

int Physical_layer_interface::send_ready()
{
	// get a pointer to the other (receive) interface to access its queue
	Physical_layer_interface *receive_interface;
	if (this == physical_layer_emulator_p->get_a_interface()) {
		receive_interface = 
		 physical_layer_emulator_p->get_b_interface();
	} else {
		receive_interface =
		 physical_layer_emulator_p->get_a_interface();
	}

	physical_layer_emulator_p->lock_queues(); // ***** LOCK

	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	int ready =
		receive_interface->queue.get_size() == 0
		||
		receive_interface->queue.get_last_release_time() <=
		 current_time;

	physical_layer_emulator_p->unlock_queues(); // ***** UNLOCK

	return ready;
}

void Physical_layer_interface::send(unsigned char byte)
{
	if (!send_ready()) {
		throw Physical_layer_exception();
	}

	// get a pointer to the other (receive) interface to access its queue
	Physical_layer_interface *receive_interface;
	if (this == physical_layer_emulator_p->get_a_interface()) {
		receive_interface = 
		 physical_layer_emulator_p->get_b_interface();
	} else {
		receive_interface =
		 physical_layer_emulator_p->get_a_interface();
	}

	physical_layer_emulator_p->lock_queues(); // ***** LOCK

	// compute release time
	struct timeval release_time;
	gettimeofday(&release_time,NULL);
	release_time += byte_delay;

	// apply corruption
	byte = corrupt.corrupt_byte(byte);

	// add byte to circular queue
	receive_interface->queue.add_last(byte, release_time);

	physical_layer_emulator_p->unlock_queues(); // ***** UNLOCK

	if (send_log) {
		cout << "PL. side " << get_a_b_string() << " sent: " <<
		 (unsigned int) byte << endl;
	}
}

int Physical_layer_interface::receive_ready()
{
	physical_layer_emulator_p->lock_queues(); // ***** LOCK

	struct timeval current_time;
	gettimeofday(&current_time, NULL);

	int ready = queue.get_size() > 0 &&
	 queue.get_first_release_time() <= current_time;

	physical_layer_emulator_p->unlock_queues(); // ***** UNLOCK

	return ready;
}

unsigned char Physical_layer_interface::receive()
{
	if (!receive_ready()) {
		throw Physical_layer_exception();
	}

	struct timeval now;

	gettimeofday(&now,NULL);

	physical_layer_emulator_p->lock_queues(); // ***** LOCK

	unsigned char byte = queue.get_first_byte();
	queue.remove_first();

	physical_layer_emulator_p->unlock_queues(); // ***** UNLOCK

	if (receive_log) {
		cout << "PL. side " << get_a_b_string() << " received: " <<
		 (unsigned int) byte << endl;
	}

	return byte;
}

// Physical_layer_emulator -----------------------------------------------

Physical_layer_emulator::Physical_layer_emulator(unsigned int bit_rate,
 Corrupt &a_corrupt, Corrupt &b_corrupt)
{
	a_interface = new Physical_layer_interface(this, bit_rate, a_corrupt);
	b_interface = new Physical_layer_interface(this, bit_rate, b_corrupt);

	pthread_mutex_init(&queue_mutex,NULL);
}

Physical_layer_interface* Physical_layer_emulator::get_a_interface()
{
	return a_interface;
}
Physical_layer_interface* Physical_layer_emulator::get_b_interface()
{
	return b_interface;
}
void Physical_layer_emulator::lock_queues(void)
{
	pthread_mutex_lock(&queue_mutex);
}
void Physical_layer_emulator::unlock_queues(void)
{
	pthread_mutex_unlock(&queue_mutex);
}
