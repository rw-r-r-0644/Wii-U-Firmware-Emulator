
#include "emulator.h"
#include "processor.h"
#include "config.h"

#include <algorithm>


Processor::Processor(Emulator *emulator, int index) {
	this->emulator = emulator;
	this->physmem = &emulator->physmem;
	this->hardware = &emulator->hardware;
	this->index = index;
	#if BREAKPOINTS
	this->breakpoints = new std::vector<uint32_t>();
	#endif
	#if WATCHPOINTS
	this->watchpoints[0][0] = new std::vector<uint32_t>();
	this->watchpoints[0][1] = new std::vector<uint32_t>();
	this->watchpoints[1][0] = new std::vector<uint32_t>();
	this->watchpoints[1][1] = new std::vector<uint32_t>();
	#endif
	enabled = false;
	paused = true;
}

Processor::Processor(const Processor &p1) {
	#if BREAKPOINTS
	this->breakpoints = new std::vector<uint32_t>(*p1.breakpoints);
	#endif
	#if WATCHPOINTS
	this->watchpoints[0][0] = new std::vector<uint32_t>(*p1.watchpoints[0][0]);
	this->watchpoints[0][1] = new std::vector<uint32_t>(*p1.watchpoints[0][1]);
	this->watchpoints[1][0] = new std::vector<uint32_t>(*p1.watchpoints[1][0]);
	this->watchpoints[1][1] = new std::vector<uint32_t>(*p1.watchpoints[1][1]);
	#endif
}

Processor::~Processor() {
	#if BREAKPOINTS
	delete breakpoints;
	#endif
	#if WATCHPOINTS
	delete this->watchpoints[0][0];
	delete this->watchpoints[0][1];
	delete this->watchpoints[1][0];
	delete this->watchpoints[1][1];
	#endif
}

void Processor::start() {
	paused = false;
	if (enabled) {
		thread = std::thread(threadFunc, this);
	}
}

void Processor::pause() {
	paused = true;
	if (thread.joinable()) {
		thread.join();
	}
}

void Processor::enable() {
	enabled = true;
	if (!paused) {
		thread = std::thread(threadFunc, this);
	}
}

void Processor::disable() {
	enabled = false;
	if (thread.joinable()) {
		thread.join();
	}
	reset();
}

void Processor::threadFunc(Processor *cpu) {
	cpu->mainLoop();
}

void Processor::mainLoop() {
	while (!paused && enabled) {
		step();
	}
}

#if BREAKPOINTS
void Processor::checkBreakpoints(uint32_t pc) {
	if (isBreakpoint(pc)) {
		Sys::out->write("Breakpoint hit at 0x%X\n", pc);
		
		emulator->signal(index);
		paused = true;
	}
}

bool Processor::isBreakpoint(uint32_t addr) {
	for (uint32_t bp : *breakpoints) {
		if (bp == addr) {
			return true;
		}
	}
	return false;
}

void Processor::addBreakpoint(uint32_t addr) {
	breakpoints->push_back(addr);
}

void Processor::removeBreakpoint(uint32_t addr) {
	breakpoints->erase(std::remove(breakpoints->begin(), breakpoints->end(), addr), breakpoints->end());
}
#endif

#if WATCHPOINTS
void Processor::checkWatchpoints(bool write, bool virt, uint32_t addr, int length) {
	if (isWatchpoint(write, virt, addr, length)) {
		Sys::out->write(
			"Watchpoint (%s) hit at %s address 0x%08X\n",
			write ? "write" : "read",
			virt ? "virtual" : "physical",
			addr
		);
		
		emulator->signal(index);
		paused = true;
	}
}

bool Processor::isWatchpoint(bool write, bool virt, uint32_t addr, int length) {
	for (uint32_t wp : *watchpoints[write][virt]) {
		if (addr <= wp && wp < addr + length) {
			return true;
		}
	}
	return false;
}

void Processor::addWatchpoint(bool write, bool virt, uint32_t addr) {
	watchpoints[write][virt]->push_back(addr);
}

void Processor::removeWatchpoint(bool write, bool virt, uint32_t addr) {
	watchpoints[write][virt]->erase(
		std::remove(watchpoints[write][virt]->begin(), watchpoints[write][virt]->end(), addr),
		watchpoints[write][virt]->end()
	);
}
#endif
