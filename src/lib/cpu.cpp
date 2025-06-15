#include "cpu.h"

#include "gameboy.h"

CPU::CPU(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts)
{
    this->restart();
}

void CPU::restart()
{
    delayIme = false;
    halted = false;
    haltBug = false;

    if(GameBoy::skipBootROM)
    {
        this->af.hi = 0x01;
        this->af.lo = 0xB0;
        this->bc.hi = 0x00;
        this->bc.lo = 0x13;
        this->de.hi = 0x00;
        this->de.lo = 0xD8;
        this->hl.hi = 0x01;
        this->hl.lo = 0x4D;
        this->pc = 0x0100;
        this->sp = 0xFFFE;
    }
    else
    {
        this->af.hi = 0x00;
        this->af.lo = 0x00;
        this->bc.hi = 0x00;
        this->bc.lo = 0x00;
        this->de.hi = 0x00;
        this->de.lo = 0x00;
        this->hl.hi = 0x00;
        this->hl.lo = 0x00;
        this->pc = 0x0000;
        this->sp = 0xFFFE;
    }
}

// =================================================================================
// Helper Functions
// =================================================================================

void CPU::setFlag(Flag flag, const bool val)
{
    if(val)    
        this->af.lo |= static_cast<u8>(flag); // Set (1)
    else
        this->af.lo &= ~static_cast<u8>(flag); // Clear (0)
}

bool CPU::getFlag(Flag flag) const
{
    return (this->af.lo & static_cast<u8>(flag)) ? true : false;
}

bool CPU::isConditionTrue(ConditionCode conditionCode) const
{
    switch(conditionCode)
    {
        case ConditionCode::None:
            return true;
            break;
        case ConditionCode::Z:
            return this->getFlag(Flag::Z);
            break;
        case ConditionCode::NZ:
            return !this->getFlag(Flag::Z);
            break;
        case ConditionCode::C:
            return this->getFlag(Flag::C);
            break;
        case ConditionCode::NC:
            return !this->getFlag(Flag::C);
            break;
    }
}

u8 CPU::fetchByte()
{
    const u8 val = this->bus.readByte(this->pc);
    ++this->pc;
    return val;
}

u16 CPU::fetchWord()
{
    const u16 val = this->bus.readWord(this->pc);
    this->pc += 2;
    return val;
}

// =================================================================================
// Instructions
// =================================================================================

// -------- 8-Bit Load ---------------

void CPU::LD(u8& dest, const u8 val)
{
    dest = val;
}

void CPU::LDMemory(const u16 addr, const u8 val)
{
    this->bus.writeByte(addr, val);
}

void CPU::LDH(const u8 offset)
{
    this->af.hi = this->bus.readByte(0xFF00 + offset);
}

void CPU::LDHMemory(const u8 offset)
{
    this->bus.writeByte(0xFF00 + offset, this->af.hi);
}

// -------- 16-Bit Load --------------

void CPU::LD(u16& dest, const u16 val)
{
    dest = val;
}

void CPU::LDMemory(const u16 addr, const u16 word)
{
    this->bus.writeWord(addr, word);
}

void CPU::LDHLAdjustedSP(const i8 offset)
{
    this->hl.pair = this->sp + offset;
    this->setFlag(Flag::Z, 0);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, ((this->sp & 0xF) + (offset & 0xF)) > 0xF);
    this->setFlag(Flag::C, ((this->sp & 0xFF) + (offset & 0xFF)) > 0xFF);
}

void CPU::PUSH(const u16 val)
{
    this->sp -= 2;
    this->bus.writeWord(this->sp, val);
}

void CPU::POP(u16& dest)
{
    const u16 val = this->bus.readWord(this->sp);
    this->sp += 2;
    dest = val;

    if(&dest == &this->af.pair)
        this->af.pair &= 0xFFF0;
}

// -------- 8-Bit Arithmetic ---------

void CPU::ADD(const u8 val, bool carry)
{
    const u16 sum = this->af.hi + val + carry;
    this->setFlag(Flag::C, sum > 0xFF);
    this->setFlag(Flag::H, ((this->af.hi & 0xF) + (val & 0xF) + carry) > 0xF);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::Z, !static_cast<u8>(sum));
    this->af.hi = static_cast<u8>(sum);
}

void CPU::SUB(const u8 val, bool carry)
{
    const u16 difference = this->af.hi - val - carry;
    this->setFlag(Flag::C, (this->af.hi < (val + carry)));
    this->setFlag(Flag::H, ((this->af.hi & 0xF) < ((val & 0xF) + carry)));
    this->setFlag(Flag::N, 1);
    this->setFlag(Flag::Z, !static_cast<u8>(difference));
    this->af.hi = static_cast<u8>(difference);
}

void CPU::CP(const u8 val)
{
    const u8 difference = this->af.hi - val;
    this->setFlag(Flag::Z, !difference);
    this->setFlag(Flag::N, 1);
    this->setFlag(Flag::H, (val & 0xF) > (this->af.hi & 0xF));
    this->setFlag(Flag::C, val > this->af.hi);
}

void CPU::INC(u8& reg)
{
    ++reg;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, !(reg & 0xF));
}

void CPU::INCMemoryHL()
{
    this->bus.writeByte(this->hl.pair, this->bus.readByte(this->hl.pair) + 1);
    const u8 result = this->bus.readByte(this->hl.pair);
    this->setFlag(Flag::Z, !result);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, !(result & 0xF));
}

void CPU::DEC(u8& reg)
{
    --reg;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 1);
    this->setFlag(Flag::H, (reg & 0xF) == 0xF);
}

void CPU::DECMemoryHL()
{
    this->bus.writeByte(this->hl.pair, this->bus.readByte(this->hl.pair) - 1);
    const u8 result = this->bus.readByte(this->hl.pair);
    this->setFlag(Flag::Z, !result);
    this->setFlag(Flag::N, 1);
    this->setFlag(Flag::H, (result & 0xF) == 0xF);
}

void CPU::AND(const u8 val)
{
    this->af.hi &= val;
    this->setFlag(Flag::Z, !this->af.hi);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 1);
    this->setFlag(Flag::C, 0);
}

void CPU::OR(const u8 val)
{
    this->af.hi |= val;
    this->setFlag(Flag::Z, !this->af.hi);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, 0);
}

void CPU::XOR(const u8 val)
{
    this->af.hi ^= val;
    this->setFlag(Flag::Z, !this->af.hi);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, 0);
}

void CPU::CCF()
{
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, !this->getFlag(Flag::C));
}

void CPU::SCF()
{
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, 1);
}

void CPU::DAA()
{
    u16 adjustment = 0;
    if(this->getFlag(Flag::N))
    {
        if(this->getFlag(Flag::H))
            adjustment |= 0x6;
        if(this->getFlag(Flag::C))
            adjustment |= 0x60;
        this->af.hi -= adjustment;
    }
    else
    {
        if(this->getFlag(Flag::H) || (this->af.hi & 0xF) > 0x9)
            adjustment |= 0x6;
        if(this->getFlag(Flag::C) || this->af.hi > 0x99)
        {
            adjustment |= 0x60;
            this->setFlag(Flag::C, 1);
        }
        this->af.hi += adjustment;
    }
    this->setFlag(Flag::Z, !this->af.hi);
    this->setFlag(Flag::H, 0);
}

void CPU::CPL()
{
    this->af.hi = ~this->af.hi;
    this->setFlag(Flag::N, 1);
    this->setFlag(Flag::H, 1);
}

// -------- 16-Bit Arithmetic --------

void CPU::INC(u16& reg)
{
    ++reg;
}

void CPU::DEC(u16& reg)
{
    --reg;
}

void CPU::ADDHL(const u16 reg)
{
    const u32 sum = this->hl.pair + reg;
    this->setFlag(Flag::C, sum > 0xFFFF);
    this->setFlag(Flag::H, ((this->hl.pair & 0xFFF) + (reg & 0xFFF)) > 0xFFF);
    this->setFlag(Flag::N, 0);
    this->hl.pair = static_cast<u16>(sum);
}

void CPU::ADDSPRelative(const i8 offset)
{
    const u32 sum = this->sp + offset;
    this->setFlag(Flag::Z, 0);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::C, ((this->sp & 0xFF) + (offset & 0xFF)) > 0xFF);
    this->setFlag(Flag::H, ((this->sp & 0xF) + (offset & 0xF)) > 0xF);
    this->sp = sum;
}

// -------- Bit Operations -----------

void CPU::RLCA()
{
    this->RLC(this->af.hi);
    this->setFlag(Flag::Z, 0);
}

void CPU::RRCA()
{
    this->RRC(this->af.hi);
    this->setFlag(Flag::Z, 0);
}

void CPU::RLA()
{
    this->RL(this->af.hi);
    this->setFlag(Flag::Z, 0);
}

void CPU::RRA()
{
    this->RR(this->af.hi);
    this->setFlag(Flag::Z, 0);
}

void CPU::RLC(u8& reg)
{
    u8 carry = (reg & 0x80) >> 7;
    this->setFlag(Flag::C, reg & 0x80);
    reg <<= 1;
    reg |= carry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !reg);
}

void CPU::RLCMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    u8 carry = (temp & 0x80) >> 7;
    this->setFlag(Flag::C, temp & 0x80);
    temp <<= 1;
    temp |= carry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !temp);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::RRC(u8& reg)
{
    u8 carry = (reg & 0x1) << 7;
    this->setFlag(Flag::C, reg & 0x1);
    reg >>= 1;
    reg |= carry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !reg);
}

void CPU::RRCMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    u8 carry = (temp & 0x1) << 7;
    this->setFlag(Flag::C, temp & 0x1);
    temp >>= 1;
    temp |= carry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !temp);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::RR(u8& reg)
{
    const u8 carry = this->getFlag(Flag::C);
    this->setFlag(Flag::C, reg & 0x1);
    reg >>= 1;
    reg |= (carry << 7);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !reg);
}

void CPU::RRMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    const u8 carry = this->getFlag(Flag::C);
    this->setFlag(Flag::C, temp & 0x1);
    temp >>= 1;
    temp |= (carry << 7);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !temp);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::RL(u8& reg)
{
    bool oldCarry = this->getFlag(Flag::C);
    this->setFlag(Flag::C, reg & 0x80);
    reg <<= 1;
    reg |= oldCarry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !reg);
}
void CPU::RLMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    bool oldCarry = this->getFlag(Flag::C);
    this->setFlag(Flag::C, temp & 0x80);
    temp <<= 1;
    temp |= oldCarry;
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::Z, !temp);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::SLA(u8& reg)
{
    this->setFlag(Flag::C, reg & 0x80);
    reg <<= 1;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
}

void CPU::SLAMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    this->setFlag(Flag::C, temp & 0x80);
    temp <<= 1;
    this->setFlag(Flag::Z, !temp);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::SRA(u8& reg)
{
    this->setFlag(Flag::C, reg & 0x1);
    const u8 msb = reg & (1 << 7);
    reg >>= 1;
    reg |= msb;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
}

void CPU::SRAMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    this->setFlag(Flag::C, temp & 0x1);
    const u8 msb = temp & (1 << 7);
    temp >>= 1;
    temp |= msb;
    this->setFlag(Flag::Z, !temp);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::SWAP(u8& reg)
{
    const u8 lower = reg << 4;
    reg = (reg >> 4) | lower;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, 0);
}

void CPU::SWAPMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    const u8 lower = temp << 4;
    temp = (temp >> 4) | lower;
    this->setFlag(Flag::Z, !temp);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->setFlag(Flag::C, 0);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::SRL(u8& reg)
{
    this->setFlag(Flag::C, reg & 0x1);
    reg >>= 1;
    this->setFlag(Flag::Z, !reg);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
}

void CPU::SRLMemoryHL()
{
    u8 temp = this->bus.readByte(this->hl.pair);
    this->setFlag(Flag::C, temp & 0x1);
    temp >>= 1;
    this->setFlag(Flag::Z, !temp);
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 0);
    this->bus.writeByte(this->hl.pair, temp);
}

void CPU::BIT(const u8 bit, const u8 val)
{
    this->setFlag(Flag::Z, !(val & bit));
    this->setFlag(Flag::N, 0);
    this->setFlag(Flag::H, 1);
}

void CPU::RES(const u8 bit, u8& reg)
{
    reg &= ~(bit);
}

void CPU::RESMemoryHL(const u8 bit)
{
    this->bus.writeByte(this->hl.pair, this->bus.readByte(this->hl.pair) & ~(bit));
}

void CPU::SET(const u8 bit, u8& reg)
{
    reg |= bit;
}

void CPU::SETMemoryHL(const u8 bit)
{
    this->bus.writeByte(this->hl.pair, this->bus.readByte(this->hl.pair) | bit);
}

// -------- Control Flow -------------

void CPU::JP(const u16 addr, ConditionCode conditionCode)
{
    if(this->isConditionTrue(conditionCode))
        this->pc = addr;
}

void CPU::JR(const i8 offset, ConditionCode conditionCode)
{
    if(this->isConditionTrue(conditionCode))
        this->pc += offset;
}

void CPU::CALL(const u16 addr, ConditionCode conditionCode)
{
    if(this->isConditionTrue(conditionCode))
    {
        this->PUSH(this->pc);
        this->pc = addr;
    }
}

void CPU::RET(ConditionCode conditionCode, bool fromInterruptHandler)
{
    if(this->isConditionTrue(conditionCode))
    {
        this->POP(this->pc);

        if(fromInterruptHandler)
            this->delayIme = true;
    }
}

void CPU::RST(const u16 vec)
{
    this->PUSH(this->pc);
    this->pc = vec;
}

// -------- Misc ---------------------

void CPU::HALT()
{
    if(!this->interrupts.ime && this->interrupts.flag & this->interrupts.enable & 0x1F)
    {
        this->halted = false;
        this->haltBug = true;
    }
    else
    {
        this->halted = true;
    }
}

void CPU::STOP()
{

}

void CPU::DI()
{
    this->delayIme = false;
    this->interrupts.ime = false;
}

void CPU::EI()
{
    this->delayIme = true;
}

// =================================================================================
// Opcodes
// =================================================================================

u8 CPU::executeOpcode(const u8 opcode)
{
    if(this->delayIme)
    {
        this->interrupts.ime = true;
        this->delayIme = false;
    }

    switch(opcode)
    {
        case 0x00: return 4; break;
        case 0x01: this->LD(this->bc.pair, this->fetchWord()); return 12; break;
        case 0x02: this->LDMemory(this->bc.pair, this->af.hi); return 8; break;
        case 0x03: this->INC(this->bc.pair); return 8; break;
        case 0x04: this->INC(this->bc.hi); return 4; break;
        case 0x05: this->DEC(this->bc.hi); return 4; break;
        case 0x06: this->LD(this->bc.hi, this->fetchByte()); return 8; break;
        case 0x07: this->RLCA(); return 4; break;
        case 0x08: this->LDMemory(this->fetchWord(), this->sp); return 20; break;
        case 0x09: this->ADDHL(this->bc.pair); return 8; break;
        case 0x0A: this->LD(this->af.hi, this->bus.readByte(this->bc.pair)); return 8; break;
        case 0x0B: this->DEC(this->bc.pair); return 8; break;
        case 0x0C: this->INC(this->bc.lo); return 4; break;
        case 0x0D: this->DEC(this->bc.lo); return 4; break;
        case 0x0E: this->LD(this->bc.lo, this->fetchByte()); return 8; break;
        case 0x0F: this->RRCA(); return 4; break;
        case 0x10: this->STOP(); return 4; break;
        case 0x11: this->LD(this->de.pair, this->fetchWord()); return 12; break;
        case 0x12: this->LDMemory(this->de.pair, this->af.hi); return 8; break;
        case 0x13: this->INC(this->de.pair); return 8; break;
        case 0x14: this->INC(this->de.hi); return 4; break;
        case 0x15: this->DEC(this->de.hi); return 4; break;
        case 0x16: this->LD(this->de.hi, this->fetchByte()); return 8; break;
        case 0x17: this->RLA(); return 4; break;
        case 0x18: this->JR(this->fetchByte(), ConditionCode::None); return 12; break;
        case 0x19: this->ADDHL(this->de.pair); return 8; break;
        case 0x1A: this->LD(this->af.hi, this->bus.readByte(this->de.pair)); return 8; break;
        case 0x1B: this->DEC(this->de.pair); return 8; break;
        case 0x1C: this->INC(this->de.lo); return 4; break;
        case 0x1D: this->DEC(this->de.lo); return 4; break;
        case 0x1E: this->LD(this->de.lo, this->fetchByte()); return 8; break;
        case 0x1F: this->RRA(); return 4; break;
        case 0x20: this->JR(this->fetchByte(), ConditionCode::NZ); return 12; break;
        case 0x21: this->LD(this->hl.pair, this->fetchWord()); return 12; break;
        case 0x22: this->LDMemory(this->hl.pair++, this->af.hi); return 8; break;
        case 0x23: this->INC(this->hl.pair); return 8; break;
        case 0x24: this->INC(this->hl.hi); return 4; break;
        case 0x25: this->DEC(this->hl.hi); return 4; break;
        case 0x26: this->LD(this->hl.hi, this->fetchByte()); return 8; break;
        case 0x27: this->DAA(); return 4; break;
        case 0x28: this->JR(this->fetchByte(), ConditionCode::Z); return 12; break;
        case 0x29: this->ADDHL(this->hl.pair); return 8; break;
        case 0x2A: this->LD(this->af.hi, this->bus.readByte(this->hl.pair++)); return 8; break;
        case 0x2B: this->DEC(this->hl.pair); return 8; break;
        case 0x2C: this->INC(this->hl.lo); return 4; break;
        case 0x2D: this->DEC(this->hl.lo); return 4; break;
        case 0x2E: this->LD(this->hl.lo, this->fetchByte()); return 8; break;
        case 0x2F: this->CPL(); return 4; break;
        case 0x30: this->JR(this->fetchByte(), ConditionCode::NC); return 12; break;
        case 0x31: this->LD(this->sp, this->fetchWord()); return 12; break;
        case 0x32: this->LDMemory(this->hl.pair--, this->af.hi); return 8; break;
        case 0x33: this->INC(this->sp); return 8; break;
        case 0x34: this->INCMemoryHL(); return 12; break;
        case 0x35: this->DECMemoryHL(); return 12; break;
        case 0x36: this->LDMemory(this->hl.pair, this->fetchByte()); return 12; break;
        case 0x37: this->SCF(); return 4; break;
        case 0x38: this->JR(this->fetchByte(), ConditionCode::C); return 12; break;
        case 0x39: this->ADDHL(this->sp); return 8; break;
        case 0x3A: this->LD(this->af.hi, this->bus.readByte(this->hl.pair--)); return 8; break;
        case 0x3B: this->DEC(this->sp); return 8; break;
        case 0x3C: this->INC(this->af.hi); return 4; break;
        case 0x3D: this->DEC(this->af.hi); return 4; break;
        case 0x3E: this->LD(this->af.hi, this->fetchByte()); return 8; break;
        case 0x3F: this->CCF(); return 4; break;
        case 0x40: this->LD(this->bc.hi, this->bc.hi); return 4; break;
        case 0x41: this->LD(this->bc.hi, this->bc.lo); return 4; break;
        case 0x42: this->LD(this->bc.hi, this->de.hi); return 4; break;
        case 0x43: this->LD(this->bc.hi, this->de.lo); return 4; break;
        case 0x44: this->LD(this->bc.hi, this->hl.hi); return 4; break;
        case 0x45: this->LD(this->bc.hi, this->hl.lo); return 4; break;
        case 0x46: this->LD(this->bc.hi, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x47: this->LD(this->bc.hi, this->af.hi); return 4; break;
        case 0x48: this->LD(this->bc.lo, this->bc.hi); return 4; break;
        case 0x49: this->LD(this->bc.lo, this->bc.lo); return 4; break;
        case 0x4A: this->LD(this->bc.lo, this->de.hi); return 4; break;
        case 0x4B: this->LD(this->bc.lo, this->de.lo); return 4; break;
        case 0x4C: this->LD(this->bc.lo, this->hl.hi); return 4; break;
        case 0x4D: this->LD(this->bc.lo, this->hl.lo); return 4; break;
        case 0x4E: this->LD(this->bc.lo, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x4F: this->LD(this->bc.lo, this->af.hi); return 4; break;
        case 0x50: this->LD(this->de.hi, this->bc.hi); return 4; break;
        case 0x51: this->LD(this->de.hi, this->bc.lo); return 4; break;
        case 0x52: this->LD(this->de.hi, this->de.hi); return 4; break;
        case 0x53: this->LD(this->de.hi, this->de.lo); return 4; break;
        case 0x54: this->LD(this->de.hi, this->hl.hi); return 4; break;
        case 0x55: this->LD(this->de.hi, this->hl.lo); return 4; break;
        case 0x56: this->LD(this->de.hi, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x57: this->LD(this->de.hi, this->af.hi); return 4; break;
        case 0x58: this->LD(this->de.lo, this->bc.hi); return 4; break;
        case 0x59: this->LD(this->de.lo, this->bc.lo); return 4; break;
        case 0x5A: this->LD(this->de.lo, this->de.hi); return 4; break;
        case 0x5B: this->LD(this->de.lo, this->de.lo); return 4; break;
        case 0x5C: this->LD(this->de.lo, this->hl.hi); return 4; break;
        case 0x5D: this->LD(this->de.lo, this->hl.lo); return 4; break;
        case 0x5E: this->LD(this->de.lo, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x5F: this->LD(this->de.lo, this->af.hi); return 4; break;
        case 0x60: this->LD(this->hl.hi, this->bc.hi); return 4; break;
        case 0x61: this->LD(this->hl.hi, this->bc.lo); return 4; break;
        case 0x62: this->LD(this->hl.hi, this->de.hi); return 4; break;
        case 0x63: this->LD(this->hl.hi, this->de.lo); return 4; break;
        case 0x64: this->LD(this->hl.hi, this->hl.hi); return 4; break;
        case 0x65: this->LD(this->hl.hi, this->hl.lo); return 4; break;
        case 0x66: this->LD(this->hl.hi, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x67: this->LD(this->hl.hi, this->af.hi); return 4; break;
        case 0x68: this->LD(this->hl.lo, this->bc.hi); return 4; break;
        case 0x69: this->LD(this->hl.lo, this->bc.lo); return 4; break;
        case 0x6A: this->LD(this->hl.lo, this->de.hi); return 4; break;
        case 0x6B: this->LD(this->hl.lo, this->de.lo); return 4; break;
        case 0x6C: this->LD(this->hl.lo, this->hl.hi); return 4; break;
        case 0x6D: this->LD(this->hl.lo, this->hl.lo); return 4; break;
        case 0x6E: this->LD(this->hl.lo, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x6F: this->LD(this->hl.lo, this->af.hi); return 4; break;
        case 0x70: this->LDMemory(this->hl.pair, this->bc.hi); return 8; break;
        case 0x71: this->LDMemory(this->hl.pair, this->bc.lo); return 8; break;
        case 0x72: this->LDMemory(this->hl.pair, this->de.hi); return 8; break;
        case 0x73: this->LDMemory(this->hl.pair, this->de.lo); return 8; break;
        case 0x74: this->LDMemory(this->hl.pair, this->hl.hi); return 8; break;
        case 0x75: this->LDMemory(this->hl.pair, this->hl.lo); return 8; break;
        case 0x76: this->HALT(); return 4; break;
        case 0x77: this->LDMemory(this->hl.pair, this->af.hi); return 8; break;
        case 0x78: this->LD(this->af.hi, this->bc.hi); return 4; break;
        case 0x79: this->LD(this->af.hi, this->bc.lo); return 4; break;
        case 0x7A: this->LD(this->af.hi, this->de.hi); return 4; break;
        case 0x7B: this->LD(this->af.hi, this->de.lo); return 4; break;
        case 0x7C: this->LD(this->af.hi, this->hl.hi); return 4; break;
        case 0x7D: this->LD(this->af.hi, this->hl.lo); return 4; break;
        case 0x7E: this->LD(this->af.hi, this->bus.readByte(this->hl.pair)); return 8; break;
        case 0x7F: this->LD(this->af.hi, this->af.hi); return 4; break;
        case 0x80: this->ADD(this->bc.hi, false); return 4; break;
        case 0x81: this->ADD(this->bc.lo, false); return 4; break;
        case 0x82: this->ADD(this->de.hi, false); return 4; break;
        case 0x83: this->ADD(this->de.lo, false); return 4; break;
        case 0x84: this->ADD(this->hl.hi, false); return 4; break;
        case 0x85: this->ADD(this->hl.lo, false); return 4; break;
        case 0x86: this->ADD(this->bus.readByte(this->hl.pair), false); return 8; break;
        case 0x87: this->ADD(this->af.hi, false); return 4; break;
        case 0x88: this->ADD(this->bc.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x89: this->ADD(this->bc.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x8A: this->ADD(this->de.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x8B: this->ADD(this->de.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x8C: this->ADD(this->hl.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x8D: this->ADD(this->hl.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x8E: this->ADD(this->bus.readByte(this->hl.pair), this->getFlag(Flag::C)); return 8; break;
        case 0x8F: this->ADD(this->af.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x90: this->SUB(this->bc.hi, false); return 4; break;
        case 0x91: this->SUB(this->bc.lo, false); return 4; break;
        case 0x92: this->SUB(this->de.hi, false); return 4; break;
        case 0x93: this->SUB(this->de.lo, false); return 4; break;
        case 0x94: this->SUB(this->hl.hi, false); return 4; break;
        case 0x95: this->SUB(this->hl.lo, false); return 4; break;
        case 0x96: this->SUB(this->bus.readByte(this->hl.pair), false); return 8; break;
        case 0x97: this->SUB(this->af.hi, false); return 4; break;
        case 0x98: this->SUB(this->bc.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x99: this->SUB(this->bc.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x9A: this->SUB(this->de.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x9B: this->SUB(this->de.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x9C: this->SUB(this->hl.hi, this->getFlag(Flag::C)); return 4; break;
        case 0x9D: this->SUB(this->hl.lo, this->getFlag(Flag::C)); return 4; break;
        case 0x9E: this->SUB(this->bus.readByte(this->hl.pair), this->getFlag(Flag::C)); return 8; break;
        case 0x9F: this->SUB(this->af.hi, this->getFlag(Flag::C)); return 4; break;
        case 0xA0: this->AND(this->bc.hi); return 4; break;
        case 0xA1: this->AND(this->bc.lo); return 4; break;
        case 0xA2: this->AND(this->de.hi); return 4; break;
        case 0xA3: this->AND(this->de.lo); return 4; break;
        case 0xA4: this->AND(this->hl.hi); return 4; break;
        case 0xA5: this->AND(this->hl.lo); return 4; break;
        case 0xA6: this->AND(this->bus.readByte(this->hl.pair)); return 8; break;
        case 0xA7: this->AND(this->af.hi); return 4; break;
        case 0xA8: this->XOR(this->bc.hi); return 4; break;
        case 0xA9: this->XOR(this->bc.lo); return 4; break;
        case 0xAA: this->XOR(this->de.hi); return 4; break;
        case 0xAB: this->XOR(this->de.lo); return 4; break;
        case 0xAC: this->XOR(this->hl.hi); return 4; break;
        case 0xAD: this->XOR(this->hl.lo); return 4; break;
        case 0xAE: this->XOR(this->bus.readByte(this->hl.pair)); return 8; break;
        case 0xAF: this->XOR(this->af.hi); return 4; break;
        case 0xB0: this->OR(this->bc.hi); return 4; break;
        case 0xB1: this->OR(this->bc.lo); return 4; break;
        case 0xB2: this->OR(this->de.hi); return 4; break;
        case 0xB3: this->OR(this->de.lo); return 4; break;
        case 0xB4: this->OR(this->hl.hi); return 4; break;
        case 0xB5: this->OR(this->hl.lo); return 4; break;
        case 0xB6: this->OR(this->bus.readByte(this->hl.pair)); return 8; break;
        case 0xB7: this->OR(this->af.hi); return 4; break;
        case 0xB8: this->CP(this->bc.hi); return 4; break;
        case 0xB9: this->CP(this->bc.lo); return 4; break;
        case 0xBA: this->CP(this->de.hi); return 4; break;
        case 0xBB: this->CP(this->de.lo); return 4; break;
        case 0xBC: this->CP(this->hl.hi); return 4; break;
        case 0xBD: this->CP(this->hl.lo); return 4; break;
        case 0xBE: this->CP(this->bus.readByte(this->hl.pair)); return 8; break;
        case 0xBF: this->CP(this->af.hi); return 4; break;
        case 0xC0: this->RET(ConditionCode::NZ, false); return 20; break;
        case 0xC1: this->POP(this->bc.pair); return 12; break;
        case 0xC2: this->JP(this->fetchWord(), ConditionCode::NZ); return 16; break;
        case 0xC3: this->JP(this->fetchWord(), ConditionCode::None); return 16; break;
        case 0xC4: this->CALL(this->fetchWord(), ConditionCode::NZ); return 24; break;
        case 0xC5: this->PUSH(this->bc.pair); return 16; break;
        case 0xC6: this->ADD(this->fetchByte(), false); return 8; break;
        case 0xC7: this->RST(0x00); return 16; break;
        case 0xC8: this->RET(ConditionCode::Z, false); return 20; break;
        case 0xC9: this->RET(ConditionCode::None, false); return 16; break;
        case 0xCA: this->JP(this->fetchWord(), ConditionCode::Z); return 16; break;
        case 0xCB: return this->executeExtendedOpcode(this->fetchByte()); break;
        case 0xCC: this->CALL(this->fetchWord(), ConditionCode::Z); return 24; break;
        case 0xCD: this->CALL(this->fetchWord(), ConditionCode::None); return 24; break;
        case 0xCE: this->ADD(this->fetchByte(), this->getFlag(Flag::C)); return 8; break;
        case 0xCF: this->RST(0x08); return 16; break;
        case 0xD0: this->RET(ConditionCode::NC, false); return 20; break;
        case 0xD1: this->POP(this->de.pair); return 12; break;
        case 0xD2: this->JP(this->fetchWord(), ConditionCode::NC); return 16; break;
        case 0xD4: this->CALL(this->fetchWord(), ConditionCode::NC); return 24; break;
        case 0xD5: this->PUSH(this->de.pair); return 16; break;
        case 0xD6: this->SUB(this->fetchByte(), false); return 8; break;
        case 0xD7: this->RST(0x10); return 16; break;
        case 0xD8: this->RET(ConditionCode::C, false); return 20; break;
        case 0xD9: this->RET(ConditionCode::None, true); return 16; break;
        case 0xDA: this->JP(this->fetchWord(), ConditionCode::C); return 16; break;
        case 0xDC: this->CALL(this->fetchWord(), ConditionCode::C); return 24; break;
        case 0xDE: this->SUB(this->fetchByte(), this->getFlag(Flag::C)); return 8; break;
        case 0xDF: this->RST(0x18); return 16; break;
        case 0xE0: this->LDHMemory(this->fetchByte()); return 12; break;
        case 0xE1: this->POP(this->hl.pair); return 12; break;
        case 0xE2: this->LDHMemory(this->bc.lo); return 8; break;
        case 0xE5: this->PUSH(this->hl.pair); return 16; break;
        case 0xE6: this->AND(this->fetchByte()); return 8; break;
        case 0xE7: this->RST(0x20); return 16; break;
        case 0xE8: this->ADDSPRelative(this->fetchByte()); return 16; break;
        case 0xE9: this->JP(this->hl.pair, ConditionCode::None); return 4; break;
        case 0xEA: this->LDMemory(this->fetchWord(), this->af.hi); return 16; break;
        case 0xEE: this->XOR(this->fetchByte()); return 8; break;
        case 0xEF: this->RST(0x28); return 16; break;
        case 0xF0: this->LDH(this->fetchByte()); return 12; break;
        case 0xF1: this->POP(this->af.pair); return 12; break;
        case 0xF2: this->LD(this->af.hi, this->bus.readByte(0xFF00 + this->bc.lo)); return 8; break;
        case 0xF3: this->DI(); return 4; break;
        case 0xF5: this->PUSH(this->af.pair); return 16; break;
        case 0xF6: this->OR(this->fetchByte()); return 8; break;
        case 0xF7: this->RST(0x30); return 16; break;
        case 0xF8: this->LDHLAdjustedSP(this->fetchByte()); return 12; break;
        case 0xF9: this->LD(this->sp, this->hl.pair); return 8; break;
        case 0xFA: this->LD(this->af.hi, this->bus.readByte(this->fetchWord())); return 16; break;
        case 0xFB: this->EI(); return 4; break;
        case 0xFE: this->CP(this->fetchByte()); return 8; break;
        case 0xFF: this->RST(0x38); return 16; break;
        default: return 4; break;
    }
}

u8 CPU::executeExtendedOpcode(const u8 opcode)
{
    switch(opcode)
    {
        case 0x00: this->RLC(this->bc.hi); return 8; break;
        case 0x01: this->RLC(this->bc.lo); return 8; break;
        case 0x02: this->RLC(this->de.hi); return 8; break;
        case 0x03: this->RLC(this->de.lo); return 8; break;
        case 0x04: this->RLC(this->hl.hi); return 8; break;
        case 0x05: this->RLC(this->hl.lo); return 8; break;
        case 0x06: this->RLCMemoryHL(); return 16; break;
        case 0x07: this->RLC(this->af.hi); return 8; break;
        case 0x08: this->RRC(this->bc.hi); return 8; break;
        case 0x09: this->RRC(this->bc.lo); return 8; break;
        case 0x0A: this->RRC(this->de.hi); return 8; break;
        case 0x0B: this->RRC(this->de.lo); return 8; break;
        case 0x0C: this->RRC(this->hl.hi); return 8; break;
        case 0x0D: this->RRC(this->hl.lo); return 8; break;
        case 0x0E: this->RRCMemoryHL(); return 16; break;
        case 0x0F: this->RRC(this->af.hi); return 8; break;
        case 0x10: this->RL(this->bc.hi); return 8; break;
        case 0x11: this->RL(this->bc.lo); return 8; break;
        case 0x12: this->RL(this->de.hi); return 8; break;
        case 0x13: this->RL(this->de.lo); return 8; break;
        case 0x14: this->RL(this->hl.hi); return 8; break;
        case 0x15: this->RL(this->hl.lo); return 8; break;
        case 0x16: this->RLMemoryHL(); return 16; break;
        case 0x17: this->RL(this->af.hi); return 8; break;
        case 0x18: this->RR(this->bc.hi); return 8; break;
        case 0x19: this->RR(this->bc.lo); return 8; break;
        case 0x1A: this->RR(this->de.hi); return 8; break;
        case 0x1B: this->RR(this->de.lo); return 8; break;
        case 0x1C: this->RR(this->hl.hi); return 8; break;
        case 0x1D: this->RR(this->hl.lo); return 8; break;
        case 0x1E: this->RRMemoryHL(); return 16; break;
        case 0x1F: this->RR(this->af.hi); return 8; break;
        case 0x20: this->SLA(this->bc.hi); return 8; break;
        case 0x21: this->SLA(this->bc.lo); return 8; break;
        case 0x22: this->SLA(this->de.hi); return 8; break;
        case 0x23: this->SLA(this->de.lo); return 8; break;
        case 0x24: this->SLA(this->hl.hi); return 8; break;
        case 0x25: this->SLA(this->hl.lo); return 8; break;
        case 0x26: this->SLAMemoryHL(); return 16; break;
        case 0x27: this->SLA(this->af.hi); return 8; break;
        case 0x28: this->SRA(this->bc.hi); return 8; break;
        case 0x29: this->SRA(this->bc.lo); return 8; break;
        case 0x2A: this->SRA(this->de.hi); return 8; break;
        case 0x2B: this->SRA(this->de.lo); return 8; break;
        case 0x2C: this->SRA(this->hl.hi); return 8; break;
        case 0x2D: this->SRA(this->hl.lo); return 8; break;
        case 0x2E: this->SRAMemoryHL(); return 16; break;
        case 0x2F: this->SRA(this->af.hi); return 8; break;
        case 0x30: this->SWAP(this->bc.hi); return 8; break;
        case 0x31: this->SWAP(this->bc.lo); return 8; break;
        case 0x32: this->SWAP(this->de.hi); return 8; break;
        case 0x33: this->SWAP(this->de.lo); return 8; break;
        case 0x34: this->SWAP(this->hl.hi); return 8; break;
        case 0x35: this->SWAP(this->hl.lo); return 8; break;
        case 0x36: this->SWAPMemoryHL(); return 16; break;
        case 0x37: this->SWAP(this->af.hi); return 8; break;
        case 0x38: this->SRL(this->bc.hi); return 8; break;
        case 0x39: this->SRL(this->bc.lo); return 8; break;
        case 0x3A: this->SRL(this->de.hi); return 8; break;
        case 0x3B: this->SRL(this->de.lo); return 8; break;
        case 0x3C: this->SRL(this->hl.hi); return 8; break;
        case 0x3D: this->SRL(this->hl.lo); return 8; break;
        case 0x3E: this->SRLMemoryHL(); return 16; break;
        case 0x3F: this->SRL(this->af.hi); return 8; break;
        case 0x40: this->BIT(1 << 0, this->bc.hi); return 8; break;
        case 0x41: this->BIT(1 << 0, this->bc.lo); return 8; break;
        case 0x42: this->BIT(1 << 0, this->de.hi); return 8; break;
        case 0x43: this->BIT(1 << 0, this->de.lo); return 8; break;
        case 0x44: this->BIT(1 << 0, this->hl.hi); return 8; break;
        case 0x45: this->BIT(1 << 0, this->hl.lo); return 8; break;
        case 0x46: this->BIT(1 << 0, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x47: this->BIT(1 << 0, this->af.hi); return 8; break;
        case 0x48: this->BIT(1 << 1, this->bc.hi); return 8; break;
        case 0x49: this->BIT(1 << 1, this->bc.lo); return 8; break;
        case 0x4A: this->BIT(1 << 1, this->de.hi); return 8; break;
        case 0x4B: this->BIT(1 << 1, this->de.lo); return 8; break;
        case 0x4C: this->BIT(1 << 1, this->hl.hi); return 8; break;
        case 0x4D: this->BIT(1 << 1, this->hl.lo); return 8; break;
        case 0x4E: this->BIT(1 << 1, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x4F: this->BIT(1 << 1, this->af.hi); return 8; break;
        case 0x50: this->BIT(1 << 2, this->bc.hi); return 8; break;
        case 0x51: this->BIT(1 << 2, this->bc.lo); return 8; break;
        case 0x52: this->BIT(1 << 2, this->de.hi); return 8; break;
        case 0x53: this->BIT(1 << 2, this->de.lo); return 8; break;
        case 0x54: this->BIT(1 << 2, this->hl.hi); return 8; break;
        case 0x55: this->BIT(1 << 2, this->hl.lo); return 8; break;
        case 0x56: this->BIT(1 << 2, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x57: this->BIT(1 << 2, this->af.hi); return 8; break;
        case 0x58: this->BIT(1 << 3, this->bc.hi); return 8; break;
        case 0x59: this->BIT(1 << 3, this->bc.lo); return 8; break;
        case 0x5A: this->BIT(1 << 3, this->de.hi); return 8; break;
        case 0x5B: this->BIT(1 << 3, this->de.lo); return 8; break;
        case 0x5C: this->BIT(1 << 3, this->hl.hi); return 8; break;
        case 0x5D: this->BIT(1 << 3, this->hl.lo); return 8; break;
        case 0x5E: this->BIT(1 << 3, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x5F: this->BIT(1 << 3, this->af.hi); return 8; break;
        case 0x60: this->BIT(1 << 4, this->bc.hi); return 8; break;
        case 0x61: this->BIT(1 << 4, this->bc.lo); return 8; break;
        case 0x62: this->BIT(1 << 4, this->de.hi); return 8; break;
        case 0x63: this->BIT(1 << 4, this->de.lo); return 8; break;
        case 0x64: this->BIT(1 << 4, this->hl.hi); return 8; break;
        case 0x65: this->BIT(1 << 4, this->hl.lo); return 8; break;
        case 0x66: this->BIT(1 << 4, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x67: this->BIT(1 << 4, this->af.hi); return 8; break;
        case 0x68: this->BIT(1 << 5, this->bc.hi); return 8; break;
        case 0x69: this->BIT(1 << 5, this->bc.lo); return 8; break;
        case 0x6A: this->BIT(1 << 5, this->de.hi); return 8; break;
        case 0x6B: this->BIT(1 << 5, this->de.lo); return 8; break;
        case 0x6C: this->BIT(1 << 5, this->hl.hi); return 8; break;
        case 0x6D: this->BIT(1 << 5, this->hl.lo); return 8; break;
        case 0x6E: this->BIT(1 << 5, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x6F: this->BIT(1 << 5, this->af.hi); return 8; break;
        case 0x70: this->BIT(1 << 6, this->bc.hi); return 8; break;
        case 0x71: this->BIT(1 << 6, this->bc.lo); return 8; break;
        case 0x72: this->BIT(1 << 6, this->de.hi); return 8; break;
        case 0x73: this->BIT(1 << 6, this->de.lo); return 8; break;
        case 0x74: this->BIT(1 << 6, this->hl.hi); return 8; break;
        case 0x75: this->BIT(1 << 6, this->hl.lo); return 8; break;
        case 0x76: this->BIT(1 << 6, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x77: this->BIT(1 << 6, this->af.hi); return 8; break;
        case 0x78: this->BIT(1 << 7, this->bc.hi); return 8; break;
        case 0x79: this->BIT(1 << 7, this->bc.lo); return 8; break;
        case 0x7A: this->BIT(1 << 7, this->de.hi); return 8; break;
        case 0x7B: this->BIT(1 << 7, this->de.lo); return 8; break;
        case 0x7C: this->BIT(1 << 7, this->hl.hi); return 8; break;
        case 0x7D: this->BIT(1 << 7, this->hl.lo); return 8; break;
        case 0x7E: this->BIT(1 << 7, this->bus.readByte(this->hl.pair)); return 16; break;
        case 0x7F: this->BIT(1 << 7, this->af.hi); return 8; break;
        case 0x80: this->RES(1 << 0, this->bc.hi); return 8; break;
        case 0x81: this->RES(1 << 0, this->bc.lo); return 8; break;
        case 0x82: this->RES(1 << 0, this->de.hi); return 8; break;
        case 0x83: this->RES(1 << 0, this->de.lo); return 8; break;
        case 0x84: this->RES(1 << 0, this->hl.hi); return 8; break;
        case 0x85: this->RES(1 << 0, this->hl.lo); return 8; break;
        case 0x86: this->RESMemoryHL(1 << 0); return 16; break;
        case 0x87: this->RES(1 << 0, this->af.hi); return 8; break;
        case 0x88: this->RES(1 << 1, this->bc.hi); return 8; break;
        case 0x89: this->RES(1 << 1, this->bc.lo); return 8; break;
        case 0x8A: this->RES(1 << 1, this->de.hi); return 8; break;
        case 0x8B: this->RES(1 << 1, this->de.lo); return 8; break;
        case 0x8C: this->RES(1 << 1, this->hl.hi); return 8; break;
        case 0x8D: this->RES(1 << 1, this->hl.lo); return 8; break;
        case 0x8E: this->RESMemoryHL(1 << 1); return 16; break;
        case 0x8F: this->RES(1 << 1, this->af.hi); return 8; break;
        case 0x90: this->RES(1 << 2, this->bc.hi); return 8; break;
        case 0x91: this->RES(1 << 2, this->bc.lo); return 8; break;
        case 0x92: this->RES(1 << 2, this->de.hi); return 8; break;
        case 0x93: this->RES(1 << 2, this->de.lo); return 8; break;
        case 0x94: this->RES(1 << 2, this->hl.hi); return 8; break;
        case 0x95: this->RES(1 << 2, this->hl.lo); return 8; break;
        case 0x96: this->RESMemoryHL(1 << 2); return 16; break;
        case 0x97: this->RES(1 << 2, this->af.hi); return 8; break;
        case 0x98: this->RES(1 << 3, this->bc.hi); return 8; break;
        case 0x99: this->RES(1 << 3, this->bc.lo); return 8; break;
        case 0x9A: this->RES(1 << 3, this->de.hi); return 8; break;
        case 0x9B: this->RES(1 << 3, this->de.lo); return 8; break;
        case 0x9C: this->RES(1 << 3, this->hl.hi); return 8; break;
        case 0x9D: this->RES(1 << 3, this->hl.lo); return 8; break;
        case 0x9E: this->RESMemoryHL(1 << 3); return 16; break;
        case 0x9F: this->RES(1 << 3, this->af.hi); return 8; break;
        case 0xA0: this->RES(1 << 4, this->bc.hi); return 8; break;
        case 0xA1: this->RES(1 << 4, this->bc.lo); return 8; break;
        case 0xA2: this->RES(1 << 4, this->de.hi); return 8; break;
        case 0xA3: this->RES(1 << 4, this->de.lo); return 8; break;
        case 0xA4: this->RES(1 << 4, this->hl.hi); return 8; break;
        case 0xA5: this->RES(1 << 4, this->hl.lo); return 8; break;
        case 0xA6: this->RESMemoryHL(1 << 4); return 16; break;
        case 0xA7: this->RES(1 << 4, this->af.hi); return 8; break;
        case 0xA8: this->RES(1 << 5, this->bc.hi); return 8; break;
        case 0xA9: this->RES(1 << 5, this->bc.lo); return 8; break;
        case 0xAA: this->RES(1 << 5, this->de.hi); return 8; break;
        case 0xAB: this->RES(1 << 5, this->de.lo); return 8; break;
        case 0xAC: this->RES(1 << 5, this->hl.hi); return 8; break;
        case 0xAD: this->RES(1 << 5, this->hl.lo); return 8; break;
        case 0xAE: this->RESMemoryHL(1 << 5); return 16; break;
        case 0xAF: this->RES(1 << 5, this->af.hi); return 8; break;
        case 0xB0: this->RES(1 << 6, this->bc.hi); return 8; break;
        case 0xB1: this->RES(1 << 6, this->bc.lo); return 8; break;
        case 0xB2: this->RES(1 << 6, this->de.hi); return 8; break;
        case 0xB3: this->RES(1 << 6, this->de.lo); return 8; break;
        case 0xB4: this->RES(1 << 6, this->hl.hi); return 8; break;
        case 0xB5: this->RES(1 << 6, this->hl.lo); return 8; break;
        case 0xB6: this->RESMemoryHL(1 << 6); return 16; break;
        case 0xB7: this->RES(1 << 6, this->af.hi); return 8; break;
        case 0xB8: this->RES(1 << 7, this->bc.hi); return 8; break;
        case 0xB9: this->RES(1 << 7, this->bc.lo); return 8; break;
        case 0xBA: this->RES(1 << 7, this->de.hi); return 8; break;
        case 0xBB: this->RES(1 << 7, this->de.lo); return 8; break;
        case 0xBC: this->RES(1 << 7, this->hl.hi); return 8; break;
        case 0xBD: this->RES(1 << 7, this->hl.lo); return 8; break;
        case 0xBE: this->RESMemoryHL(1 << 7); return 16; break;
        case 0xBF: this->RES(1 << 7, this->af.hi); return 8; break;
        case 0xC0: this->SET(1 << 0, this->bc.hi); return 8; break;
        case 0xC1: this->SET(1 << 0, this->bc.lo); return 8; break;
        case 0xC2: this->SET(1 << 0, this->de.hi); return 8; break;
        case 0xC3: this->SET(1 << 0, this->de.lo); return 8; break;
        case 0xC4: this->SET(1 << 0, this->hl.hi); return 8; break;
        case 0xC5: this->SET(1 << 0, this->hl.lo); return 8; break;
        case 0xC6: this->SETMemoryHL(1 << 0); return 16; break;
        case 0xC7: this->SET(1 << 0, this->af.hi); return 8; break;
        case 0xC8: this->SET(1 << 1, this->bc.hi); return 8; break;
        case 0xC9: this->SET(1 << 1, this->bc.lo); return 8; break;
        case 0xCA: this->SET(1 << 1, this->de.hi); return 8; break;
        case 0xCB: this->SET(1 << 1, this->de.lo); return 8; break;
        case 0xCC: this->SET(1 << 1, this->hl.hi); return 8; break;
        case 0xCD: this->SET(1 << 1, this->hl.lo); return 8; break;
        case 0xCE: this->SETMemoryHL(1 << 1); return 16; break;
        case 0xCF: this->SET(1 << 1, this->af.hi); return 8; break;
        case 0xD0: this->SET(1 << 2, this->bc.hi); return 8; break;
        case 0xD1: this->SET(1 << 2, this->bc.lo); return 8; break;
        case 0xD2: this->SET(1 << 2, this->de.hi); return 8; break;
        case 0xD3: this->SET(1 << 2, this->de.lo); return 8; break;
        case 0xD4: this->SET(1 << 2, this->hl.hi); return 8; break;
        case 0xD5: this->SET(1 << 2, this->hl.lo); return 8; break;
        case 0xD6: this->SETMemoryHL(1 << 2); return 16; break;
        case 0xD7: this->SET(1 << 2, this->af.hi); return 8; break;
        case 0xD8: this->SET(1 << 3, this->bc.hi); return 8; break;
        case 0xD9: this->SET(1 << 3, this->bc.lo); return 8; break;
        case 0xDA: this->SET(1 << 3, this->de.hi); return 8; break;
        case 0xDB: this->SET(1 << 3, this->de.lo); return 8; break;
        case 0xDC: this->SET(1 << 3, this->hl.hi); return 8; break;
        case 0xDD: this->SET(1 << 3, this->hl.lo); return 8; break;
        case 0xDE: this->SETMemoryHL(1 << 3); return 16; break;
        case 0xDF: this->SET(1 << 3, this->af.hi); return 8; break;
        case 0xE0: this->SET(1 << 4, this->bc.hi); return 8; break;
        case 0xE1: this->SET(1 << 4, this->bc.lo); return 8; break;
        case 0xE2: this->SET(1 << 4, this->de.hi); return 8; break;
        case 0xE3: this->SET(1 << 4, this->de.lo); return 8; break;
        case 0xE4: this->SET(1 << 4, this->hl.hi); return 8; break;
        case 0xE5: this->SET(1 << 4, this->hl.lo); return 8; break;
        case 0xE6: this->SETMemoryHL(1 << 4); return 16; break;
        case 0xE7: this->SET(1 << 4, this->af.hi); return 8; break;
        case 0xE8: this->SET(1 << 5, this->bc.hi); return 8; break;
        case 0xE9: this->SET(1 << 5, this->bc.lo); return 8; break;
        case 0xEA: this->SET(1 << 5, this->de.hi); return 8; break;
        case 0xEB: this->SET(1 << 5, this->de.lo); return 8; break;
        case 0xEC: this->SET(1 << 5, this->hl.hi); return 8; break;
        case 0xED: this->SET(1 << 5, this->hl.lo); return 8; break;
        case 0xEE: this->SETMemoryHL(1 << 5); return 16; break;
        case 0xEF: this->SET(1 << 5, this->af.hi); return 8; break;
        case 0xF0: this->SET(1 << 6, this->bc.hi); return 8; break;
        case 0xF1: this->SET(1 << 6, this->bc.lo); return 8; break;
        case 0xF2: this->SET(1 << 6, this->de.hi); return 8; break;
        case 0xF3: this->SET(1 << 6, this->de.lo); return 8; break;
        case 0xF4: this->SET(1 << 6, this->hl.hi); return 8; break;
        case 0xF5: this->SET(1 << 6, this->hl.lo); return 8; break;
        case 0xF6: this->SETMemoryHL(1 << 6); return 16; break;
        case 0xF7: this->SET(1 << 6, this->af.hi); return 8; break;
        case 0xF8: this->SET(1 << 7, this->bc.hi); return 8; break;
        case 0xF9: this->SET(1 << 7, this->bc.lo); return 8; break;
        case 0xFA: this->SET(1 << 7, this->de.hi); return 8; break;
        case 0xFB: this->SET(1 << 7, this->de.lo); return 8; break;
        case 0xFC: this->SET(1 << 7, this->hl.hi); return 8; break;
        case 0xFD: this->SET(1 << 7, this->hl.lo); return 8; break;
        case 0xFE: this->SETMemoryHL(1 << 7); return 16; break;
        case 0xFF: this->SET(1 << 7, this->af.hi); return 8; break;
        default: return 8; break;
    }
}

// =================================================================================
// Main Logic
// =================================================================================

u8 CPU::step()
{
    if(this->halted)
        return 4;

    const u8 opcode = this->fetchByte();

    if(this->haltBug)
    {
        --this->pc;
        this->haltBug = false;
    }

    const u8 cycles = this->executeOpcode(opcode);

    return cycles;
}