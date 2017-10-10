


all:
	g++ -g -I./ -L./ -Wl,-rpath=/usr/local/gcc-7.1.0/lib64/ *.cpp -lcurl -lstdc++ -lpthread -o test
