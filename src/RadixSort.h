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
	 gl::Program blockscan;
	 gl::Buffer buffer;
	 gl::Buffer prefixsums;
	 gl::Buffer blocksums;
	 gl::Buffer blocksumsums;
};

#endif /* !defined RADIXSORT_H */
