#ifndef RADIXSORT_H
#define RADIXSORT_H

#include "common.h"
#include "GpuAlg.h"

class RadixSort : public GpuAlg
{
public:
	 RadixSort (void);
	 virtual ~RadixSort (void);
	 virtual void Run (void);
private:
	 gl::Program counting;
	 gl::Buffer buffer;
	 gl::Buffer prefixsums;
	 gl::Buffer blocksums;
};

#endif /* !defined RADIXSORT_H */
