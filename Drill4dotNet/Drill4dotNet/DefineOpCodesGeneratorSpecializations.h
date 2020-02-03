
// Designed to use together with opcode.def from the .net source.
// The opcode.def calls OPDEF macro for each Intermediate Language
// instruction, allowing to generate desired declarations.
// Unfortunately, the list includes some artificial entries, like CEE_ILLEGAL.
// This file provides means to exclude such entries.
// User of this class defines macro OPDEF_REAL_INSTRUCTION the same way as the was
// defining OPDEF. Then he writes:
//     #include "DefineOpCodesGeneratorSpecializations.h"
//     #include <opcode.def>
//     #include "UnDefineOpCodesGeneratorSpecializations.h"
// and this will only invoke OPDEF_REAL_INSTRUCTION for real instructions.

#ifndef OPDEF_SPECIALIZATIONS_DEFINED

#define OPDEF_LENGTH_SPECIALIZATION_0(\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) 

#define OPDEF_LENGTH_SPECIALIZATION_1(\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) \
OPDEF_REAL_INSTRUCTION(\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) 

#define OPDEF_LENGTH_SPECIALIZATION_2(\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) \
OPDEF_REAL_INSTRUCTION(\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) 

#define OPDEF( \
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior) \
OPDEF_LENGTH_SPECIALIZATION_ ## codeLength (\
    canonicalName, \
    stringName, \
    stackPop, \
    stackPush, \
    inlineArgumentType, \
    operationKind, \
    codeLength, \
    byte1, \
    byte2, \
    controlBehavior)

#define OPDEF_SPECIALIZATIONS_DEFINED

#endif
