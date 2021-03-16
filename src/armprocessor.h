
#pragma once

#include "processor.h"
#include "armcore.h"
#include "armmmu.h"
#include "armjit.h"
#include "armthumbjit.h"
#include "logger.h"
#include "enum.h"
#include "config.h"

#include <cstdint>


class Emulator;

class ARMProcessor : public Processor {
public:
	enum ConditionType {
		EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL
	};
	
	ARMProcessor(Emulator *emulator);
	
	template <class T>
	bool read(uint32_t addr, T *value) {
		#if STATS
		dataReads++;
		#endif
		
		#if WATCHPOINTS
		checkWatchpoints(false, true, addr, sizeof(T));
		#endif
		
		bool supervisor = core.getMode() != ARMCore::User;
		if (!mmu.translate(&addr, MemoryAccess::DataRead, supervisor)) {
			core.triggerException(ARMCore::DataAbort);
			return false;
		}
		
		#if WATCHPOINTS
		checkWatchpoints(false, false, addr, sizeof(T));
		#endif
		
		*value = physmem->read<T>(addr);
		return true;
	}
	
	template <class T>
	bool write(uint32_t addr, T value) {
		#if STATS
		dataWrites++;
		#endif
		
		#if WATCHPOINTS
		checkWatchpoints(true, true, addr, sizeof(T));
		#endif
		
		bool supervisor = core.getMode() != ARMCore::User;
		if (!mmu.translate(&addr, MemoryAccess::DataWrite, supervisor)) {
			core.triggerException(ARMCore::DataAbort);
			return false;
		}
		
		#if WATCHPOINTS
		checkWatchpoints(true, false, addr, sizeof(T));
		#endif
		
		physmem->write<T>(addr, value);
		return true;
	}
	
	void reset();
	void step();
	
	bool coprocessorRead(int coproc, int opc, uint32_t *value, int rn, int rm, int type);
	bool coprocessorWrite(int coproc, int opc, uint32_t value, int rn, int rm, int type);
	
	ARMCore core;
	ARMMMU mmu;
	ARMJIT jit;
	ARMThumbJIT thumbJit;
	
	#if STATS
	uint64_t thumbInstrs;
	uint64_t armInstrs;
	uint64_t dataReads;
	uint64_t dataWrites;
	#endif
	
	bool printpc = false;
	
private:
	void stepThumb();
	void stepARM();
	void updateTimer();
	
	void checkDebugPoints();
	void checkInterrupts();
	
	int timer;
	
	ConsoleLogger printer;
	FileLogger console;
	FileLogger syslog;
};
