//===----------------------------------------------------------------------===//
// SystemS profiles and nodes
//===----------------------------------------------------------------------===//

def systems_ret      : SDNode<"SystemSISD::RET", SDTNone,
                            [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "SystemSInstrFormats.td"

//===----------------------------------------------------------------------===//
// SystemS Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

class SImmAsmOperand<int width> : AsmOperandClass {
  let Name = "SImm" # width;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

def systemsm16 : Operand<i32>, ImmLeaf<i32, [{return isInt<16>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<16>;
  let EncoderMethod = "getSImm16OpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SYSTEMSM16";
  let OperandNamespace = "SystemSOp";
}

//===----------------------------------------------------------------------===//
// SystemS Instructions
//===----------------------------------------------------------------------===//

let
  hasSideEffects = 0, 
  mayLoad = 0,
  mayStore = 0,
  isCall = 1,
  isBranch = 1,
  isTerminator = 1,
  r2 = 0,
  r3_imm = 0 in
def BR : SystemSInst<
  0xBB,
  (outs),
  (ins GPR:$r1),
  "br $r1",
  [(brind GPR:$r1)]
>;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoRET : SystemSPseudoInst<
  (outs),
  (ins),
  "# pseudo_ret",
  [(systems_ret)]
>, PseudoInstExpansion<(BR R0)>;

let isAsCheapAsAMove = 1, isCodeGenOnly = 1, isReMaterializable = 1, r2 = 0 in
def MOVLI : SystemSInst<
  0xAA,
  (outs GPR:$r1),
  (ins systemsm16:$r3_imm),
  "li $r1 $r3_imm",
  [(set GPR:$r1, systemsm16:$r3_imm)]
>;

// Fixed-point math instructions
let r3_imm = 0 in {
def FPSQRT : SystemSInst<
  0xC0,
  (outs GPR:$r1),
  (ins GPR:$r2),
  "fpsqrt $r1, $r2",
  [(set GPR:$r1, (int_systems_fpsqrt GPR:$r2))]
>;

def FPSIN : SystemSInst<
  0xC1,
  (outs GPR:$r1),
  (ins GPR:$r2),
  "fpsin $r1, $r2",
  [(set GPR:$r1, (int_systems_fpsin GPR:$r2))]
>;

def FPCOS : SystemSInst<
  0xC2,
  (outs GPR:$r1),
  (ins GPR:$r2),
  "fpcos $r1, $r2",
  [(set GPR:$r1, (int_systems_fpcos GPR:$r2))]
>;
} // End let r3_imm = 0

def FPATAN2 : SystemSInst<
  0xC2,
  (outs GPR:$r1),
  (ins GPR:$r2, GPR:$r3_imm),
  "fpatan2 $r1, $r2, $r3_imm",
  [(set GPR:$r1, (int_systems_fpatan2 GPR:$r2, GPR:$r3_imm))]
>;

// Graphics instructions
def PUTPIXEL : SystemSInst<
  0xEE,
  (outs),
  (ins GPR:$r1, GPR:$r2, GPR:$r3_imm),
  "putpixel $r1, $r2, $r3_imm",
  [(int_systems_putpixel GPR:$r1, GPR:$r2, GPR:$r3_imm)]
>;

let r2 = 0, r3_imm = 0 in
def FLUSH : SystemSInst<
  0xEF,
  (outs GPR:$r1),
  (ins),
  "flush $r1",
  [(set GPR:$r1, (int_systems_flush))]
>;

// Utility instructions
let r2 = 0, r3_imm = 0 in {
def LOG : SystemSInst<
  0xF0,
  (outs),
  (ins GPR:$r1),
  "log $r1",
  [(int_systems_log GPR:$r1)]
>;

def DELTATIME : SystemSInst<
  0xF1,
  (outs GPR:$r1),
  (ins),
  "deltatime $r1",
  [(set GPR:$r1, (int_systems_deltatime))]
>;
} // End let r2 = 0, r3_imm = 0