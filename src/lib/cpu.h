#pragma once

#include "types.h"
#include "bus.h"
#include "interrupts.h"

class CPU
{
    private:
        enum class Flag : u8
        {
            Z = 0x80,
            N = 0x40,
            H = 0x20,
            C = 0x10,
        };

        enum class ConditionCode : u8
        {
            None,
            Z,
            NZ,
            C,
            NC,
        };

        union Register
        {
            struct
            {
                u8 lo;
                u8 hi;
            };
            u16 pair;
        };

        Register af;
        Register bc;
        Register de;
        Register hl;

        u16 sp;
        u16 pc;

        bool delayIme;

        bool halted;
        bool haltBug;

        Interrupts& interrupts;
        Bus& bus;

        friend class GameBoy;
        friend class Interrupts;

        void setFlag(Flag flag, const bool val);
        bool getFlag(Flag flag) const;

        bool isConditionTrue(ConditionCode conditionCode) const;

        u8 fetchByte();
        u16 fetchWord();

        // -------- 8-Bit Load ---------------

        void LD(u8& dest, const u8 val);
        void LDMemory(const u16 addr, const u8 val);
        void LDH(const u8 offset);
        void LDHMemory(const u8 offset);

        // -------- 16-Bit Load --------------

        void LD(u16& dest, const u16 val);
        void LDMemory(const u16 addr, const u16 word);
        void LDHLAdjustedSP(const i8 offset);
        void PUSH(const u16 val);
        void POP(u16& dest);

        // -------- 8-Bit Arithmetic ---------

        void ADD(const u8 val, bool carry);
        void SUB(const u8 val, bool carry);
        void CP(const u8 val);
        void INC(u8& reg);
        void INCMemoryHL();
        void DEC(u8& reg);
        void DECMemoryHL();
        void AND(const u8 val);
        void OR(const u8 val);
        void XOR(const u8 val);
        void CCF();
        void SCF();
        void DAA();
        void CPL();

        // -------- 16-Bit Arithmetic --------

        void INC(u16& reg);
        void DEC(u16& reg);
        void ADDHL(const u16 reg);
        void ADDSPRelative(const i8 offset);

        // -------- Bit Operations -----------

        void RLCA();
        void RRCA();
        void RLA();
        void RRA();
        void RLC(u8& reg);
        void RLCMemoryHL();
        void RRC(u8& reg);
        void RRCMemoryHL();
        void RR(u8& reg);
        void RRMemoryHL();
        void RL(u8& reg);
        void RLMemoryHL();
        void SLA(u8& reg);
        void SLAMemoryHL();
        void SRA(u8& reg);
        void SRAMemoryHL();
        void SWAP(u8& reg);
        void SWAPMemoryHL();
        void SRL(u8& reg);
        void SRLMemoryHL();
        void BIT(const u8 bit, const u8 val);
        void RES(const u8 bit, u8& reg);
        void RESMemoryHL(const u8 bit);
        void SET(const u8 bit, u8& reg);
        void SETMemoryHL(const u8 bit);

        // -------- Control Flow -------------

        void JP(const u16 addr, ConditionCode conditionCode);
        void JR(const i8 offset, ConditionCode conditionCode);
        void CALL(const u16 addr, ConditionCode conditionCode);
        void RET(ConditionCode conditionCode, bool fromInterruptHandler);
        void RST(const u16 vec);

        // -------- Misc ---------------------

        void HALT();
        void STOP();
        void DI();
        void EI();

        u8 executeOpcode(const u8 opcode);
        u8 executeExtendedOpcode(const u8 opcode);
    
    public:
        CPU(Bus& bus, Interrupts& interrupts);

        void restart();

        u8 step();
};