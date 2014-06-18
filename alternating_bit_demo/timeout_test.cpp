#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "link_layer.h"

using namespace std;

unsigned int BIT_RATE = 800; // 100B/s

Corrupt no_corrupt = Corrupt();
Physical_layer_emulator
 physical_layer_emulator(BIT_RATE, no_corrupt, no_corrupt);

Link_layer* a_link_layer = new Link_layer
 (physical_layer_emulator.get_a_interface(), 3000000);

Link_layer* b_link_layer = new Link_layer
 (physical_layer_emulator.get_b_interface(), 3000000);

int main(int argc,char* argv[])
{
	unsigned char send_a = 100;
	unsigned char send_b = 101;
	unsigned char receive;

	// a: send
	cout << "main. sending a." << endl;
	a_link_layer->send(&send_a,1);

	// wait to see timeout resends
	cout << "Enter 'x', followed by Enter, to continue: ";
	cin >> receive;

	// b: send
	cout << "main. sending b." << endl;
	b_link_layer->send(&send_b,1);

	sleep(10);

	return 0;
}
