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
	 void SortBits (int bits);
	 void Check (void);
	 union {
		 struct {
			 GLuint counting;
			 GLuint blockscan;
			 GLuint globalsort;
			 GLuint addblocksum;
		 };
		 GLuint programs[4];
	 };
	 union {
		 struct {
			 GLuint buffer;
			 GLuint prefixsums;
			 GLuint result;
		 };
		 GLuint buffers[3];
	 };
	 std::vector<GLuint> blocksums;

	 uint32_t blocksize;
	 uint32_t numblocks;
	 int counting_bitshift;
	 int globalsort_bitshift;

	 GLuint query;

	 std::vector<glm::uvec4> data;
};

#endif /* !defined RADIXSORT_H */
