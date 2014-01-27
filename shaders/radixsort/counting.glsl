// header is included here

layout (local_size_x = HALFBLOCKSIZE) in;

layout (std430, binding = 0) readonly buffer Data
{
	uvec4 data[];
};

layout (std430, binding = 1) writeonly buffer PrefixSum
{
	uint prefixsum[];
};

layout (std430, binding = 2) writeonly buffer BlockSum
{
	uint blocksum[];
};

uniform uvec4 blocksumoffsets;

shared uvec4 mask[BLOCKSIZE];
shared uvec4 sblocksum;

const int n = BLOCKSIZE;

uniform int bitshift;

#define DONT_SHUFFLE

void main (void)
{
	const int gid = int (gl_GlobalInvocationID.x);
	const int lid = int (gl_LocalInvocationIndex);
	
	uvec4 data1 = data[2 * gid];
	uvec4 data2 = data[2 * gid + 1];
	uint bits1 = (data1.x & (3 << bitshift)) >> bitshift;
	uint bits2 = (data2.x & (3 << bitshift)) >> bitshift;
	mask[2 * lid] = uvec4 (equal (bits1 * uvec4 (1, 1, 1, 1), uvec4 (0, 1, 2, 3)));
	mask[2 * lid + 1] = uvec4 (equal (bits2 * uvec4 (1, 1, 1, 1), uvec4 (0, 1, 2, 3)));

	int offset = 1;	
	for (int d = n >> 1; d > 0; d >>= 1)
	{
		barrier ();
		memoryBarrierShared ();
		
		if (lid < d)
		{
			int ai = offset * (2 * lid + 1) - 1;
			int bi = offset * (2 * lid + 2) - 1;

			mask[bi] += mask[ai];
		}
		offset *= 2;
	}

	barrier ();
	memoryBarrierShared ();
	
	if (lid == 0)
	{
		uvec4 tmp;
		tmp.x = 0;
		tmp.y = mask[n - 1].x;
		tmp.z = tmp.y + mask[n - 1].y;
		tmp.w = tmp.z + mask[n - 1].z;
		sblocksum = tmp;

		for (int i = 0; i < 4; i++)
		{
			blocksum[blocksumoffsets[i] + gl_WorkGroupID.x] = mask[n - 1][i];
		}

		mask[n - 1] = uvec4 (0, 0, 0, 0);
	}
	
	
	for (int d = 1; d < n; d *= 2)
	{
		offset >>= 1;
		barrier ();
		memoryBarrierShared ();
		
		if (lid < d)
		{
			int ai = offset * (2 * lid + 1) - 1;
			int bi = offset * (2 * lid + 2) - 1;
			
			uvec4 tmp = mask[ai];
			mask[ai] = mask[bi];
			mask[bi] += tmp;
		}
	}
	
	barrier ();
	memoryBarrierShared ();
	
#ifdef DONT_SHUFFLE
	prefixsum[2 * gid] = mask[2 * lid][bits1];
	prefixsum[2 * gid + 1] = mask[2 * lid + 1][bits2];
#else
	uint o1 = BLOCKSIZE * gl_WorkGroupID.x + mask[2 * lid][bits1] + sblocksum[bits1];
	uint o2 = BLOCKSIZE * gl_WorkGroupID.x + mask[2 * lid + 1][bits2] + sblocksum[bits2];

	prefixsum[o1] = mask[2 * lid][bits1];
	prefixsum[o2] = mask[2 * lid + 1][bits2];

	data[o1] = data1;
	data[o2] = data2;
#endif
}
