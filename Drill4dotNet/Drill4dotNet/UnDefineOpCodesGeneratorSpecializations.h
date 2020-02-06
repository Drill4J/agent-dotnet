
// Works in conjunction with DefineOpCodesGeneratorSpecializations.h
// It undefines macros, previously defined in DefineOpCodesGeneratorSpecializations.h before including opcode.def
//
// Warning: DO NOT put #pragma once to this file!
// It is designed to be included several times and alter macros at each inclusion.
//

#ifdef OPDEF_SPECIALIZATIONS_DEFINED

#undef OPDEF_LENGTH_SPECIALIZATION_0
#undef OPDEF_LENGTH_SPECIALIZATION_1
#undef OPDEF_LENGTH_SPECIALIZATION_2
#undef OPDEF

#undef OPDEF_SPECIALIZATIONS_DEFINED

#endif
