#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "link_layer.h"

using namespace std;

unsigned int BIT_RATE = 80; // 10 Bps

Corrupt no_corrupt = Corrupt();
Physical_layer_emulator
 physical_layer_emulator(BIT_RATE, no_corrupt, no_corrupt);

Link_layer* a_link_layer = new Link_layer
 (physical_layer_emulator.get_a_interface(), 0);

Link_layer* b_link_layer = new Link_layer
 (physical_layer_emulator.get_b_interface(), 0);

void send_one(unsigned char *send_buffer, unsigned int length)
{
	// wait for send ready
	while (!a_link_layer->send_ready()) {
		cout << "main. not send_ready ..." << endl;
		usleep(500000);
	}

	// send
	cout << "main: sending: {";
	for (unsigned int i = 0; i < length; i++) {
		cout << (unsigned int)send_buffer[i] << ",";
	}
	cout << "}" << endl;
	a_link_layer->send(send_buffer, length);

	// wait for receive ready
	while (!b_link_layer->receive_ready()) {
		cout << "main: not receive_ready ..." << endl;
		usleep(500000);
	}

	// receive
	unsigned char receive_buffer[100];
	unsigned int n = b_link_layer->receive(receive_buffer);
	cout << "main. received: {";
	for (unsigned int i = 0; i < n; i++) {
		cout << (unsigned int)receive_buffer[i] << ",";
	}
	cout << "}" << endl;
}

int main(int argc, char* argv[])
{
	unsigned char buffer[] = {100, 17, 101};

	send_one(buffer, sizeof(buffer));

	sleep(5);

	return 0;
}
