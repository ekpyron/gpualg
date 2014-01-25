#version 430 core

layout (local_size_x = 128) in;

layout (std430, binding = 0) buffer Data
{
	uint data[];
};

layout (std430, binding = 1) buffer PrefixSum
{
	uint prefixsum[];
};

layout (std430, binding = 2) buffer BlockSum0
{
	uint blocksum0[];
};

layout (std430, binding = 3) buffer BlockSum1
{
	uint blocksum1[];
};

layout (std430, binding = 4) buffer BlockSum2
{
	uint blocksum2[];
};

layout (std430, binding = 5) buffer BlockSum3
{
	uint blocksum3[];
};

shared uvec4 mask[256];
shared uvec4 sblocksum;

const int n = 256;

const uvec4 bitmask[4] = {
	uvec4 (1, 0, 0, 0),
	uvec4 (0, 1, 0, 0),
	uvec4 (0, 0, 1, 0),
	uvec4 (0, 0, 0, 1)
};

void main (void)
{
	const int gid = int (gl_GlobalInvocationID.x);
	const int lid = int (gl_LocalInvocationIndex);
	
	uint data1 = data[2 * gid];
	uint data2 = data[2 * gid + 1];
	uint bits1 = data1 & 3;
	uint bits2 = data2 &3;
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

		blocksum0[gl_WorkGroupID.x] = mask[n - 1].x;
		blocksum1[gl_WorkGroupID.x] = mask[n - 1].y;
		blocksum2[gl_WorkGroupID.x] = mask[n - 1].z;
		blocksum3[gl_WorkGroupID.x] = mask[n - 1].w;

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

	prefixsum[2 * gid] = mask[2 * lid][bits1] + sblocksum[bits1];
	prefixsum[2 * gid + 1] = mask[2 * lid + 1][bits2] + sblocksum[bits2];
	
	data[mask[2 * lid][bits1] + sblocksum[bits1]] = data1;
	data[mask[2 * lid + 1][bits2] + sblocksum[bits2]] = data2;

}
