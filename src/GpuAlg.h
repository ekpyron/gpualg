#ifndef GPUALG_H
#define GPUALG_H

#include "common.h"

class GpuAlg
{
public:
	virtual ~GpuAlg (void) { }
	virtual void Run (void) = 0;
};

#endif /* GPUALG_H */
