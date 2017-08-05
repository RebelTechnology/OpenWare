#ifndef __PROGRAM_VECTOR_H
#define __PROGRAM_VECTOR_H

#include "device.h"

#if defined OWL_TESSERACT || defined OWL_MICROLAB || defined OWL_QUADFM
#include "ProgramVectorV15.h"
#endif

#if defined OWL_PEDAL || defined OWL_MODULAR
#include "ProgramVectorV13.h"
#endif

#if defined OWL_PLAYERF7 || defined  OWL_PLAYERF4
#include "ProgramVectorV14.h"
#endif

#endif /* __PROGRAM_VECTOR_H */
