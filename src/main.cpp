#include <fstream>

#include "logger/Log.hpp"
#include "devices/Ram.hpp"

#include "Interrupt.hpp"
#include "SystemBus.hpp"
#include "Cpu65816.hpp"
#include "Cpu65816Debugger.hpp"

#define LOG_TAG "MAIN"
#define MAX_BPS 100

NativeModeInterrupts nativeInterrupts{
    .coProcessorEnable = 0x0000,
    .brk = 0x0000,
    .abort = 0x0000,
    .nonMaskableInterrupt = 0x0000,
    .reset = 0x0000,
    .interruptRequest = 0x0000,

};

EmulationModeInterrupts emulationInterrupts{
    .coProcessorEnable = 0x0000,
    .unused = 0x0000,
    .abort = 0x0000,
    .nonMaskableInterrupt = 0x0000,
    .reset = 0x0000,
    .brkIrq = 0x0000
};

int bp_count = 0;
Address breakpoints[MAX_BPS];

using namespace std;

void dump_memory(Ram *ram, const char *filename) {
  ofstream dump(filename);
  uint8_t data;

  for (int i = 0; i <= 0xFFFF; i++) {
      data = ram->readByte(Address(0x00, i));
      dump.write((char *) &data, 1);
  }

  dump.close();
}

void load_memory(Ram *ram, const char *filename) {
  ifstream dump(filename);
  char data;

  for (int i = 0; i <= 0xFFFF; i++) {
    dump.read(&data, 1);
    ram->storeByte(Address(0x00, i), (uint8_t) data);
  }

  dump.close();
}

void load_program(Ram *ram, const char *filename) {
  ifstream program(filename);

  bool eof = false;

  string line;

  uint8_t size;
  uint16_t address;
  uint8_t type;

  uint8_t data;

  while (!eof && getline(program, line)) {
    size = stoi(line.substr(1, 2), nullptr, 16);
    address = stoi(line.substr(3, 4), nullptr, 16);
    type = stoi(line.substr(7, 2), nullptr, 16);

    switch (type) {
      case 0:
        for (int i = 0; i < size; i++) {
          data = stoi(line.substr(9 + 2 * i, 2), nullptr, 16);
          ram->storeByte(Address(0x00, address), data);
          address++;
        }
        break;
      case 1:
        eof = true;
        break;
      default:
        break;
    }
  }

  program.close();
}

void dump_cpu(Cpu65816 *cpu) {

}

bool add_breakpoint(Address addr) {
  if (bp_count >= MAX_BPS) {
    cout << "Cannot exceed maximum of " << MAX_BPS << " breakpoints.";
    return false;
  }

  breakpoints[bp_count] = addr;
  bp_count++;
  return true;
}

bool remove_breakpoint(Address addr) {
  bool cascade = false;
  for (int i = 0; i < bp_count; i++) {
    if (breakpoints[i].getOffset() == addr.getOffset()) {
      cascade = true;
    }
    if (cascade && i < bp_count - 1) {
      breakpoints[i] = breakpoints[i + 1];
    }
  }
  bp_count--;
  return cascade;
}

string formatAddress(Address addr) {
  char buf[8];
  snprintf(buf, 8, "%02X:%04X", addr.getBank(), addr.getOffset());
  return {buf};
}

bool breakPointHit(Cpu65816 *cpu) {
  Address curr = cpu->getProgramAddress();

  for (int i = 0; i < bp_count; i++) {
    if (curr.getOffset() == breakpoints[i].getOffset()) {
      return true;
    }
  }

  return false;
}

uint8_t getOpCode(Cpu65816 *cpu, Ram *ram) {
  Address address = cpu->getProgramAddress();
  return ram->readByte(address);
}

bool isCall(uint8_t opcode) { return opcode == 0x20 || opcode == 0x22 || opcode == 0xFC; }
bool isReturn(uint8_t opcode) { return opcode == 0x60 || opcode == 0x6B; }

int main(int argc, char *argv[]) {
  if (argc > 2) {
    Log::err(LOG_TAG).str("Usage: db658116 [working_directory]").show();
    return 1;
  }

  char *directory = argv[1];

  Ram ram = Ram(0x2);

  SystemBus systemBus = SystemBus();
  systemBus.registerDevice(&ram);

  Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
  Cpu65816Debugger debugger(cpu);

  debugger.doBeforeStep([]() {});
  debugger.doAfterStep([]() {});

  bool endPointHit = false;
  debugger.onBreakPoint([&endPointHit]() {
    endPointHit = true;
  });

  string command;
  string op;
  string param;

  bool input;
  bool bp_en = true;

  uint8_t inst;
  int depth;

  cout << endl;

  while (command != "exit") {
    cout << "[" << formatAddress(cpu.getProgramAddress()) << "] db65816> ";
    getline(cin, command);

    stringstream args(command);

    args >> op;
    input = (bool) (args >> param);

    if (op == "help") {
      cout << endl;
      cout << "ld.mem [filename]: load the entirety of RAM from a specified binary file or mem.bin" << endl;
      cout << "ld.program [filename]: load an assembled program from a specified .hex file or program.hex" << endl;
      cout << "run address: jump to the given address and start the debugger (until next breakpoint)" << endl;
      cout << "jump address: jump to the given address without running the debugger" << endl;
      cout << "continue: start or resume the debugger from the current location (until next breakpoint)" << endl;
      cout << "step: step over current function" << endl;
      cout << "step.in: step into current function" << endl;
      cout << "step.out: step out of current function" << endl;
      cout << "bp.enable: enable all breakpoints" << endl;
      cout << "bp.disable: disable all breakpoints" << endl;
      cout << "bp.add address: add a breakpoint at the given address" << endl;
      cout << "bp.remove address: remove a breakpoint at the given address" << endl;
      cout << "bp.list: list all currently active breakpoints" << endl;
      cout << "bp.clear: clear all breakpoints" << endl;
      cout << "dump.mem [filename]: dump memory to specified file or mem.bin" << endl;
      cout << "dump.cpu: dump cpu registers and status" << endl;
      cout << "exit: quit this debugger" << endl;
      cout << "help: display this message" << endl;
      cout << endl;
    }
    if (op == "ld.mem") {
      const char *name = input ? param.c_str() : "mem.bin";
      load_memory(&ram, name);
      cout << "Loaded " << name << endl;
    }
    if (op == "ld.program") {
      const char *name = input ? param.c_str() : "program.hex";
      load_program(&ram, name);
      cout << "Loaded program " << name << endl;
    }
    if (op == "jump" || op == "run") {
      if (!input) {
        cout << "Please provide an address for this command." << endl;
      } else {
        uint16_t offset = stoi(param, nullptr, 16);
        Address addr = Address(0x00, offset);
        cpu.setProgramAddress(addr);
        cout << "Will now continue from $" << formatAddress(addr) << endl;

        endPointHit = false;
      }
    }
    if (op == "continue" || op == "step.in") {
      debugger.step();
    }
    if (op == "continue" || op == "run") {
      while (!endPointHit && (!bp_en || !breakPointHit(&cpu))) {
        debugger.step();
      }
    }
    if (op == "step") {
      depth = 0;
    }
    if (op == "step.out") {
      depth = 1;
    }
    if (op == "step" || op == "step.out") {
      do {
        inst = getOpCode(&cpu, &ram);
        debugger.step();

        if (isCall(inst)) depth++;
        if (isReturn(inst)) depth--;
      } while (depth > 0);
    }
    if (op == "bp.enable") {
      bp_en = true;
      cout << bp_count << " Breakpoints enabled" << endl;
    }
    if (op == "bp.disable") {
      bp_en = false;
      cout << bp_count << " Breakpoints disabled" << endl;
    }
    if (op == "bp.add") {
      uint16_t offset = stoi(param, nullptr, 16);
      Address addr = Address(0x00, offset);
      if (add_breakpoint(addr)) {
        cout << "Breakpoint added at $" << formatAddress(addr) << endl;
      }
    }
    if (op == "bp.remove") {
      uint16_t offset = stoi(param, nullptr, 16);
      Address addr = Address(0x00, offset);
      if (remove_breakpoint(addr)) {
        cout << "Successfully removed breakpoint at $" << formatAddress(addr) << endl;
      } else {
        cout << "No breakpoint found at $" << formatAddress(addr) << endl;
      }
    }
    if (op == "bp.list") {
      cout << endl;
      cout << "ALL BREAKPOINTS:" << endl;

      if (bp_count == 0) {
        cout << "No breakpoints" << endl;
      }
      for (int i = 0; i < bp_count; i++) {
        cout << "bp" << i << ": $" << formatAddress(breakpoints[i]) << endl;
      }

      cout << endl;
      cout << "Breakpoints are currently " << (bp_en ? "ENABLED" : "DISABLED") << endl;
      cout << endl;
    }
    if (op == "bp.clear") {
      bp_count = 0;
    }
    if (op == "dump.mem") {
      const char *name = input ? param.c_str() : "mem.bin";
      dump_memory(&ram, name);
      cout << "Memory saved to " << name << endl;
    }
    if (op == "dump.cpu") {
      debugger.dumpCpu();
    }
  }

  return 0;
}
