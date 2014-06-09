#include <iostream>
#include <pthread.h>
#include "sys/time.h"

using namespace std;

class Physical_layer_emulator;

// Physical_layer_exception ---------------------------------------------

class Physical_layer_exception: public exception {
};

// Corrupt ---------------------------------------------------------------

class Corrupt {
public:
	enum {MAXIMUM_CORRUPT_LENGTH = 50};

	Corrupt();
	Corrupt(double*, unsigned int);

	unsigned char corrupt_byte(unsigned char);
private:
	double probabilities[MAXIMUM_CORRUPT_LENGTH];
	unsigned int length, index;
	unsigned int rand_state;
};

// Overflow_queue ----------------------------------------------

// *** overflow queue: circular, with release time per element
// if queue_size > 0
//	queue[queue_first]: oldest item
//	queue[queue_last]: newest item
//	queue_release_time: time to release queue[queue_last]
// if full: discard oldest item
// pre: must be locked before use
class Overflow_queue {
public:
	enum {MAXIMUM_QUEUE_LENGTH = 10};
	Overflow_queue();
	void add_last(unsigned char, struct timeval release_time);
	void remove_first();
	unsigned int get_size();
	unsigned char get_first_byte();
	struct timeval get_first_release_time();
	unsigned char get_last_byte();
	struct timeval get_last_release_time();
	void print();
private:
	struct queue_element {
		unsigned char c;
		struct timeval release_time;
	};
	queue_element queue[MAXIMUM_QUEUE_LENGTH];
	unsigned int size, first, last;
	struct timeval release_time;
};

// Physical_layer_interface ----------------------------------------------

class Physical_layer_interface {
public:
	Physical_layer_interface(Physical_layer_emulator*,
		unsigned int bit_rate, Corrupt&);

	char get_a_b_string();

	int send_ready();
	void send(unsigned char);

	int receive_ready();
	unsigned char receive();

	Overflow_queue queue; // to provide debug access to print
private:
	Physical_layer_emulator *physical_layer_emulator_p;

	unsigned int send_log, receive_log;
	struct timeval byte_delay;
	Corrupt corrupt;
	// Overflow_queue queue;
};

// Physical_layer_emulator -----------------------------------------------

class Physical_layer_emulator {
public:
	Physical_layer_emulator(unsigned int bit_rate,
	 Corrupt &a_corrupt, Corrupt &b_corrupt);

	Physical_layer_interface *get_a_interface(void);

	Physical_layer_interface *get_b_interface(void);

	void lock_queues();
	void unlock_queues();
private:
	pthread_mutex_t queue_mutex;

	Physical_layer_interface *a_interface, *b_interface;
};
