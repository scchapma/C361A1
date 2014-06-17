#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "link_layer.h"

using namespace std;

unsigned int BIT_RATE = 800; // 100B/s

double corrupt_array[] = {
	1.0,0.0, 0.0,0.0, 0.0,0.0, 0.0,0.0, 0.0,0.0, 0.0,0.0,
};

Corrupt corrupt = Corrupt(corrupt_array, sizeof(corrupt_array)/sizeof(double));
Corrupt no_corrupt = Corrupt();
Physical_layer_emulator
 physical_layer_emulator(BIT_RATE, corrupt, no_corrupt);

Link_layer* a_link_layer = new Link_layer
 (physical_layer_emulator.get_a_interface(), 3000000);

Link_layer* b_link_layer = new Link_layer
 (physical_layer_emulator.get_b_interface(), 3000000);

int main(int argc,char* argv[])
{
	unsigned char buffer[100];

	unsigned char send_a = 0;
	unsigned char send_b = 0;
	unsigned char receive_a = 0;
	unsigned char receive_b = 0;

	while (1) {
		// a: receive
		if (a_link_layer->receive_ready()) {
			unsigned int n = a_link_layer->receive(buffer);
			receive_a = buffer[0];
			cout << "main. received a: " <<
			 n << ',' << (unsigned int)receive_a << endl;
		}
		// b: receive
		if (b_link_layer->receive_ready()) {
			unsigned int n = b_link_layer->receive(buffer);
			receive_b = buffer[0];
			cout << "main. received b: " <<
			 n << ',' << (unsigned int)receive_b << endl;
		}
		// a: send
		if (a_link_layer->send_ready()) {
			cout << "main. sending a: " <<
			 (unsigned int) send_a << endl;
			a_link_layer->send(&send_a,1);
			send_a++;
		}
		// b: send
		if (b_link_layer->send_ready()) {
			cout << "main. sending b: " <<
			 (unsigned int) send_b << endl;
			b_link_layer->send(&send_b,1);
			send_b++;
		}
	}

	return 0;
}
