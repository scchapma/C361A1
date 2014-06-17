echo ---------- checksum demo
g++ -g -o checksum_demo \
	physical_layer.cpp link_layer.cpp checksum_demo.cpp -lpthread

echo ---------- simple_two_way_test
g++ -g -o simple_two_way_test \
	physical_layer.cpp link_layer.cpp simple_two_way_test.cpp -lpthread

echo ---------- timeout_test
g++ -g -o timeout_test \
	physical_layer.cpp link_layer.cpp timeout_test.cpp -lpthread
