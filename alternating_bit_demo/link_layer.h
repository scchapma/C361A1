#include <pthread.h>
#include <unistd.h>
#include <exception>

#include "physical_layer.h"

unsigned short checksum(unsigned short* buffer,unsigned int length);

class Link_layer_exception: public exception {
};

class Link_layer {
public:
	enum { FLAG_BYTE = 3, ESCAPE_BYTE = 16, MAXIMUM_DATA_LENGTH = 50 };

	Link_layer(
	 // from Physical_layer_emulator
	 Physical_layer_interface* physical_layer_interface,

	 // in microseconds
	 unsigned int timeout);

	/*
	* normal case
	*	return true if send is ready to accept a byte array
	* exceptions
	*	none
	*/
	int send_ready(void);

	/*
	* normal case
	*	send buffer to the physical layer
	* exceptions
	*	throw Link_layer_exception if send_ready returns false
	*/
	void send(unsigned char buffer[], unsigned int length);

	/*
	* normal case
	*	return true if receive has a byte array available
	* exceptions
	*	none
	*/
	int receive_ready(void);

	/*
	* normal case
	*	copy the available byte array to buffer; return its length
	* exceptions
	*	throw Link_layer_exception if receive_ready returns false
	*/
	unsigned int receive(unsigned char buffer[]);

private:
	struct packet {
		unsigned short checksum;
		unsigned char seq;
		unsigned char ack;
		unsigned char length;
		unsigned char data[Link_layer::MAXIMUM_DATA_LENGTH];
	};
	enum {PACKET_HEADER_LENGTH = 5};

	void receive_byte(void);

	void send_byte(void);

	void resend(void);

	static void* loop(void* link_layer);

	Physical_layer_interface* physical_layer_interface;

	pthread_t thread;

	pthread_mutex_t send_mutex;
	unsigned char send_buffer[100];
	unsigned int send_buffer_length, send_buffer_next;

	pthread_mutex_t receive_mutex;
	unsigned char receive_buffer[100];
	unsigned int receive_buffer_length;
	unsigned int receive_buffer_ready;
	unsigned int receive_frame_state;
	enum {IN, OUT, ESCAPED, FAIL};
};
