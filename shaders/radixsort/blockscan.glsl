#version 430 core

layout (local_size_x = 128) in;

layout (std430, binding = 2) buffer Data
{
	uint data[];
};

layout (std430, binding = 3) buffer BlockSums
{
	uint blocksums[];
};

shared uint temp[256];

const int n = 256;

void main (void)
{
	int gid = int (gl_GlobalInvocationID.x);
	int lid = int (gl_LocalInvocationIndex);
	int offset = 1;
	
	temp[2 * lid] = data[2 * gid];
	temp[2 * lid + 1] = data[2 * gid + 1];
	
	for (int d = n >> 1; d > 0; d >>= 1)
	{
		barrier ();
		memoryBarrierShared ();
		
		if (lid < d)
		{
			int ai = offset * (2 * lid + 1) - 1;
			int bi = offset * (2 * lid + 2) - 1;
			
			temp[bi] += temp[ai]; 
		}
		offset *= 2;
	}
	
	if (lid == 0)
	{
		blocksums[gl_WorkGroupID.x] = temp[n - 1];
		temp[n - 1] = 0;
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
			
			uint t = temp[ai];
			temp[ai] = temp[bi];
			temp[bi] += t;
		}
	}
	
	barrier ();
	memoryBarrierShared ();
	
	data[2 * gid] = temp[2 * lid];
	data[2 * gid + 1] = temp[2 * lid + 1];
}
