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
	 union {
		 struct {
			 GLuint blockscan;
			 GLuint addblocksum;
		 };
		 GLuint programs[2];
	 };
	 union {
		 struct {
			 GLuint buffer;
			 GLuint blocksums;
			 GLuint unused;
		 };
		 GLuint buffers[3];
	 };
};

#endif /* !defined PREFIXSUM_H */
