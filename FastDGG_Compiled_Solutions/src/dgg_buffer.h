#ifndef _DGG_BUFFER_H_
#define _DGG_BUFFER_H_
#include <fstream>
using namespace std;

struct DGGBuffer{
	char* buf;
	int len;
	int capacity;
	ofstream output_file;
	DGGBuffer() {
		buf = nullptr;
		len = 0;
		capacity = 0;
	}
	DGGBuffer(const string& output_filename) {
		output_file.open(output_filename.c_str(), ios::out | ios::binary);
		capacity = 16 * 1024 * 1024;
		buf = new char[capacity];
		len = 0;
	}
	void open(const string& output_filename) {
		output_file.open(output_filename.c_str(), ios::out | ios::binary);
		capacity = 16 * 1024 * 1024;
		buf = new char[capacity];
		len = 0;
	}

	void addStruct(const void* const ptr, int struct_size)
	{
		//fwrite(ptr, struct_size, 1, file);
		//return;
		if (len + struct_size > capacity) {
			//fwrite(buf, sizeof(char), len, file);
			output_file.write(buf, len);
			len = 0;
		}
		if (struct_size > capacity) {
			fprintf(stderr, "str too large!");
			exit(1);
		}
		memcpy((void*)(buf + len), ptr, struct_size);
		len += struct_size;
	}

	void close() {
		if (len != 0) {
			output_file.write(buf, len);
			len = 0;
		}
		output_file.close();
	}

	~DGGBuffer() {
		if (buf != nullptr) {
			delete[] buf;
		}
	}
};

#endif