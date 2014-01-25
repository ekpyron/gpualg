#ifndef PREFIXSUM_H
#define PREFIXSUM_H

#include "common.h"
#include "GpuAlg.h"

class PrefixSum : public GpuAlg
{
public:
	 PrefixSum (void);
	 virtual ~PrefixSum (void);
	 virtual void Run (void);
private:
	 gl::Program blockscan;
	 gl::Program addblocksum;
	 gl::Buffer buffer;
	 gl::Buffer blocksums;
	 gl::Buffer unused;
};

#endif /* !defined PREFIXSUM_H */
