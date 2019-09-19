
#pragma once

#include <cstdint>

class Endian {
public:
	static void swap(uint8_t *);
	static void swap(uint16_t *);
	static void swap(uint32_t *);
	static void swap(uint64_t *);
	static uint8_t swap8(uint8_t);
	static uint16_t swap16(uint16_t);
	static uint32_t swap32(uint32_t);
	static uint64_t swap64(uint64_t);
	
	template <class T>
	static void swap(T *value) {
		switch(sizeof(T)) {
			case 1:
				swap((uint8_t *)value);
				break;
			case 2:
				swap((uint16_t *)value);
				break;
			case 4:
				swap((uint32_t *)value);
				break;
			case 8:
				swap((uint64_t *)value);
				break;
		}
	}
};
