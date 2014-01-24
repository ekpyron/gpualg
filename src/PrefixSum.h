#ifndef PREFIXSUM_H
#define PREFIXSUM_H

#include <oglp/oglp.h>

class PrefixSum
{
public:
	 PrefixSum (void);
	 ~PrefixSum (void);
	 void Run (void);
	 void Frame (void);
private:
	 gl::Program blockscan;
	 gl::Program addblocksum;
	 gl::Buffer buffer;
	 gl::Buffer blocksums;
	 gl::Buffer unused;
};

#endif /* !defined PREFIXSUM_H */
