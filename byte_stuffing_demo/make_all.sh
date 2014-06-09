echo ---------- basic_test
g++ -g -o basic_test \
	physical_layer.cpp link_layer.cpp basic_test.cpp -lpthread

echo ---------- flag_test
g++ -g -o flag_test \
	physical_layer.cpp link_layer.cpp flag_test.cpp -lpthread

echo ---------- escape_test
g++ -g -o escape_test \
	physical_layer.cpp link_layer.cpp escape_test.cpp -lpthread

echo ---------- corrupted_flag_test
g++ -g -o corrupted_flag_test \
	physical_layer.cpp link_layer.cpp corrupted_flag_test.cpp -lpthread

echo ---------- corrupted_escape_test
g++ -g -o corrupted_escape_test \
	physical_layer.cpp link_layer.cpp corrupted_escape_test.cpp -lpthread
