#include "link_layer.h"
#include "timeval_operators.h"

// standard Internet checksum algorithm
unsigned short checksum(unsigned short* buffer,unsigned int length)
{
	unsigned long sum = 0;

	// sum the byte-pairs
	while (length > 1) {
		sum += *buffer++;
		length -= 2;
	}

	// handle the trailing byte, if present
	if (length == 1) {
		sum += *(unsigned char*) buffer;
	}

	// adjust for one's complement
	sum = (sum >> 16)+(sum & 0xffff);
	sum = (~(sum+(sum >> 16)) & 0xffff);

	return (unsigned short) sum;
}

// ---------------------------------- constructor

Link_layer::Link_layer(Physical_layer_interface* physical_layer_interface,
 unsigned int timeout)
{
	this->physical_layer_interface = physical_layer_interface;

	send_buffer_length = 0;
	receive_frame_state = OUT;

	pthread_mutex_init(&send_mutex,NULL);
	pthread_mutex_init(&receive_mutex,NULL);
	if (pthread_create(&thread,NULL,&Link_layer::loop,this) < 0) {
		throw Link_layer_exception();
	}
}

// ---------------------------------- send

int Link_layer::send_ready()
{
	pthread_mutex_lock(&send_mutex);

	int n = send_buffer_length == 0;

	pthread_mutex_unlock(&send_mutex);

	return n;
}

void Link_layer::send(unsigned char buffer[],unsigned int length)
{
	pthread_mutex_lock(&send_mutex);

	unsigned int i;

	send_buffer[0] = FLAG_BYTE; // start flag
    int j = 1;
	for (i = 0; i < length; i++) {
        if(buffer[i] == ESCAPE_BYTE){
            send_buffer[j] = ESCAPE_BYTE;
            send_buffer[j+1] = ESCAPE_BYTE;
            j += 2;
        }else if (buffer[i] == FLAG_BYTE){
            send_buffer[j] = ESCAPE_BYTE;
            send_buffer[j+1] = FLAG_BYTE;
            j += 2;
        }else{
            send_buffer[j] = buffer[i];
            j++;
        }
	}
	send_buffer[j] = FLAG_BYTE; // end flag
	send_buffer_length = j+1;
	send_buffer_next = 0;

	pthread_mutex_unlock(&send_mutex);
}


// purpose:
//	send next byte in send_buffer
// preconditions:
//	none
void Link_layer::send_byte(void)
{
	pthread_mutex_lock(&send_mutex);

	if (send_buffer_next == send_buffer_length) {
		pthread_mutex_unlock(&send_mutex);
		return;
	}

	if (!physical_layer_interface->send_ready()) {
		pthread_mutex_unlock(&send_mutex);
		return;
	}

	physical_layer_interface->send(send_buffer[send_buffer_next]);
	send_buffer_next++;

	pthread_mutex_unlock(&send_mutex);
}

// ---------------------------------- receive

int Link_layer::receive_ready(void)
{
	pthread_mutex_lock(&receive_mutex);

	int n = receive_buffer_ready;

	pthread_mutex_unlock(&receive_mutex);

	return n;
}

unsigned int Link_layer::receive(unsigned char buffer[])
{
	pthread_mutex_lock(&receive_mutex);

	unsigned int n = receive_buffer_length;
	for (unsigned int i = 0; i < n; i++) {
		buffer[i] = receive_buffer[i];
	}

	pthread_mutex_unlock(&receive_mutex);

	return n;
}

void Link_layer::receive_byte(void)
{
	pthread_mutex_lock(&receive_mutex);

	if (!physical_layer_interface->receive_ready()) { // no byte available
		pthread_mutex_unlock(&receive_mutex);
		return;
	}

	unsigned char b = physical_layer_interface->receive();

	if (receive_frame_state == OUT) {
		if (b == FLAG_BYTE) { // start frame
			receive_frame_state = IN;
			receive_buffer_length = 0;
		}
	} else if (receive_frame_state == IN) {
		if (b == FLAG_BYTE) { // end frame
			receive_frame_state = OUT;
			receive_buffer_ready = 1;
		} else if (b == ESCAPE_BYTE){
            receive_frame_state = ESCAPED;
        } else { // add b to receive_buffer
			receive_buffer[receive_buffer_length] = b;
			receive_buffer_length++;
		}
	} else if (receive_frame_state == ESCAPED) {
        if (b == FLAG_BYTE || b == ESCAPE_BYTE){
            receive_frame_state = IN;
            receive_buffer[receive_buffer_length] = b;
			receive_buffer_length++;
            
        } else {  // else - return - not legal string
            receive_frame_state = FAIL;
        }
    } else{  //non-legal string (receive_frame_state == FAIL
        // buffer length to 0.
        receive_buffer_length = 0;
        receive_buffer_ready = 1;
    }
	pthread_mutex_unlock(&receive_mutex);
}


void Link_layer::resend(void)
{
	// coming soon
}

// ---------------------------------- Physical Layer loop

void* Link_layer::loop(void* thread_creator)
{
	Link_layer* link_layer = ((Link_layer*) thread_creator);

	while (true) {
		// process next byte coming in from physical layer
		link_layer->receive_byte();

		// resend send_packet on timeout
		link_layer->resend();

		// send next byte from send_buffer
		link_layer->send_byte();

		// pause to allow other threads access
		usleep(1000);
	}

	return NULL;
}
