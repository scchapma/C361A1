#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "link_layer.h"

using namespace std;

struct packet {
	unsigned short checksum;
	unsigned char seq;
	unsigned char ack;
	unsigned char length;
	unsigned char data[Link_layer::MAXIMUM_DATA_LENGTH];
};
enum {PACKET_HEADER_LENGTH = 5};

int main(int argc,char* argv[])
{
	// assign values to all packet fields
	struct packet p;
	p.checksum = 0;
	p.seq = 0;
	p.ack = 1;
	p.length = 2;
	p.data[0] = 100;
	p.data[1] = 101;

	// compute checksum
	//	note: checksum length parameter is in bytes
	unsigned short c = checksum
		((unsigned short*) &p, PACKET_HEADER_LENGTH+p.length);

	cout << "checksum: " << c << endl;

	// assign checksum to packet, then transmit
	p.checksum = c;

	// receiver: save checksum, assign to 0, recompute

	return 0;
}
