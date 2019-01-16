/******************************
 * Michael Davis
 * CS 3339 - spring 2017
 * Section 252
 * Project 4
 ******************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;
    stat.clock();
    fetch();
    decode();
    execute();
    mem();
    writeback();
    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  //opcode = /* FIXME */
  //rs = /* FIXME */
  //rt = /* FIXME */
  //rd = /* FIXME */
  //shamt = /* FIXME */
  //funct = /* FIXME */
  //uimm = /* FIXME */
  //simm = /* FIXME */
  //addr = /* FIXME */

  opcode = instr & 0xfc000000;
  opcode >>= 26;
  rs = instr & 0x03e00000;
  rs >>= 21;
  rt = instr & 0x001f0000;
  rt >>= 16;
  rd = instr & 0x0000f800;
  rd >>= 11;
  shamt = instr & 0x000007c0;
  shamt >>= 6;
  funct = instr & 0x0000003f;
  uimm = instr & 0x0000ffff;
  simm = (instr & 0x0000ffff);
  simm <<= 16;
  simm >>= 16;
  addr = instr & 0x03ffffff;

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!

  opIsLoad = opIsStore = opIsMultDiv = false;
  aluOp = ADD;
  writeDest = false;
  destReg = regFile[REG_ZERO];//ALU if statement will not let it write to this register
  aluSrc1 = aluSrc2 = 0;
  storeData = 0;

  //enum vals for reference
  //enum ALU_OP { ADD, AND, SHF_L, SHF_R, CMP_LT, MUL, DIV };

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   writeDest = true;
                   destReg = rd;
                   stat.registerSrc(rs, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[0]++;
                   break;
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   writeDest = true;
                   destReg = rd;
                   stat.registerSrc(rs, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[1]++;
                   break;
        case 0x08: D(cout << "jr " << regNames[rs]);
                   pc = regFile[rs];
                   aluOp = ADD;
                   aluSrc1 = pc;
                   aluSrc2 = regFile[REG_ZERO];
                   stat.registerSrc(rs, ID);
                   stat.flush(2);
                   stat.countBranch();
                   stat.countTaken();
                   //tally[2]++;
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   regFile[rd] = hi;
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[REG_ZERO];
                   stat.registerSrc(REG_HILO, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[3]++;
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   regFile[rd] = lo;
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[REG_ZERO];
                   stat.registerSrc(REG_HILO, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[4]++;
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = MUL;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   stat.registerSrc(rs, EXE1);
                   stat.registerSrc(rt, EXE1);
                   stat.registerDest(REG_HILO, WB);
                   //tally[5]++;
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = DIV;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   stat.registerSrc(rs, EXE1);
                   stat.registerSrc(rt, EXE1);
                   stat.registerDest(REG_HILO, WB);
                   //tally[6]++;
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   writeDest = true;
                   destReg = rd;
                   stat.registerSrc(rs, EXE1);
                   stat.registerSrc(rt, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[7]++;
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = -regFile[rt];
                   writeDest = true;
                   destReg = rd;
                   stat.registerSrc(rs, EXE1);
                   stat.registerSrc(rt, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[8]++;
                   break;
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   writeDest = true;
                   destReg = rd;
                   stat.registerSrc(rs, EXE1);
                   stat.registerSrc(rt, EXE1);
                   stat.registerDest(rd, MEM1);
                   //tally[9]++;
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               stat.flush(2);
               stat.countBranch();
               stat.countTaken();
               //tally[10]++;
               break;
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               stat.flush(2);
               stat.registerDest(REG_RA, ID);
               stat.countBranch();
               stat.countTaken();
               //tally[11]++;
               break;
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               if(regFile[rs]==regFile[rt]){
                 pc = pc + (simm << 2);
                 stat.flush(2);
                 stat.countTaken();
               }
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               stat.registerSrc(rs, ID);
               stat.registerSrc(rt, ID);
               stat.countBranch();
               //tally[12]++;
               break;
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               if(regFile[rs]!=regFile[rt]){
                 pc = pc + (simm << 2);
                 stat.flush(2);
                 stat.countTaken();
               }
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               stat.registerSrc(rs, ID);
               stat.registerSrc(rt, ID);
               stat.countBranch();
               //tally[13]++;
               break;
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               aluSrc2 = simm;
               writeDest = true;
               destReg = rt;
               stat.registerSrc(rs, EXE1);
               stat.registerDest(rt, MEM1);
               //tally[14]++;
               break;
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               aluOp = AND;
               aluSrc1 = regFile[rs];
               aluSrc2 = uimm;
               writeDest = true;
               destReg = rt;
               stat.registerSrc(rs, EXE1);
               stat.registerDest(rt, MEM1);
               //tally[15]++;
               break;
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               aluOp = ADD;
               aluSrc1 = regFile[REG_ZERO];
               aluSrc2 = simm << 16;
               writeDest = true;
               destReg = rt;
               stat.registerSrc(rt, EXE1);
               stat.registerDest(rt, MEM1);
               //tally[16]++;
               break;
    case 0x1a: D(cout << "trap " << hex << addr);     //idk what trap does
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs];
                           stat.registerSrc(rs, EXE1);
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt];
                           stat.registerDest(rt, MEM1);
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               //tally[17]++;
               break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               aluOp = ADD;
               opIsLoad = true;
               aluSrc1 = simm;
               aluSrc2 = regFile[rs];
               writeDest = true;
               destReg = rt;
               stat.registerSrc(rs, EXE1);
               stat.registerDest(rt, WB);
               stat.countMemOp();
               //tally[18]++;
               break;
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               opIsStore = true;
               aluOp = ADD;
               aluSrc1 = simm;
               aluSrc2 = regFile[rs];
               storeData = regFile[rt];
               stat.registerSrc(rt, MEM1);
               stat.registerSrc(rs, EXE1);
               stat.countMemOp();
               //tally[19]++;
               break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad)
    writeData = dMem.loadWord(aluOut);
  else
    writeData = aluOut;

  if(opIsStore)
    dMem.storeWord(storeData, aluOut);
}
void CPU::writeback() {
  if(writeDest && destReg > 0) // never write to reg 0
    regFile[destReg] = writeData;

  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}
void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}
void CPU::printFinalStats() {
    float CPI = stat.getCycles(),
            ins = instructions;
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl << endl;
    cout << "cycles: " << stat.getCycles() << endl;
    cout << "CPI: " << fixed << setprecision(2) << CPI/ins << endl << endl;

    cout << "bubbles: " << stat.getBubbles() << endl;
    cout << "flushes: " << stat.getFlushes() << endl << endl;

    CPI = stat.getTotalHazards();

    cout << "RAW hazards: " << stat.getTotalHazards() << "(1 per every ";
    cout << fixed << setprecision(2) << ins/CPI << " instructions)" << endl;
    cout << " On EXE1 op: " << stat.getHazards(3) << "(" << stat.getHazards(3)/CPI * 100 << "%)" << endl;
    cout << " On EXE2 op: " << stat.getHazards(4) << "(" << stat.getHazards(4)/CPI * 100 << "%)" << endl;
    cout << " On MEM1 op: " << stat.getHazards(5) << "(" << stat.getHazards(5)/CPI * 100 << "%)" << endl;
    cout << " On MEM2 op: " << stat.getHazards(6) << "(" << stat.getHazards(6)/CPI * 100 << "%)" << endl;
    cout << endl;

    //cout << "tally:" << endl;

    //for(int i = 0; i < 20; i++){

     //cout << i << " ----- " << tally[i] << endl;}
}
