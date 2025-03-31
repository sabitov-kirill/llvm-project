include "llvm/Target/Target.td"

include "SystemSRegisterInfo.td"
include "SystemSInstrInfo.td"


class Proc<
  string Name,
  list<SubtargetFeature> Features
> : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def SystemSInstrInfo : InstrInfo;
def SystemS : Target {
  let InstructionSet = SystemSInstrInfo;
}