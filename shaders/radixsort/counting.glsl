// header is included here

layout (local_size_x = HALFBLOCKSIZE) in;

layout (std430, binding = 0) buffer Data
{
	uvec4 data[];
};

layout (std430, binding = 1) buffer Test
{
	uvec4 prefixsum[];
};

layout (std430, binding = 2) buffer BlockSum
{
	uvec4 blocksum[];
};

uniform uint numblocks;

shared uint mask[BLOCKSIZE * 4];
shared uvec4 sblocksum;

const int n = BLOCKSIZE;

uniform int bitshift;

void main (void)
{
	const int gid = int (gl_GlobalInvocationID.x);
	const int lid = int (gl_LocalInvocationIndex);
	
	uvec4 data1 = data[2 * gid];
	uvec4 data2 = data[2 * gid + 1];
	uint bits1 = (data1.x & (3 << bitshift)) >> bitshift;
	uint bits2 = (data2.x & (3 << bitshift)) >> bitshift;
	
	for (int i = 0; i < 4; i++)
	{
		if (bits1 == i)
		{
			mask[4 * 2 * lid + i] = 1;
		}
		else
		{
			mask[4 * 2 * lid + i] = 0;
		}
		if (bits2 == i)
		{
			mask[4 * (2 * lid + 1) + i] = 1;
		}
		else
		{
			mask[4 * (2 * lid + 1) + i] = 0;
		}
	}
	
	memoryBarrierShared ();
	

	int offset = 1;	
	for (int d = n >> 1; d > 0; d >>= 1)
	{
		barrier ();
		memoryBarrierShared ();
		
		if (lid < d)
		{
			int ai = offset * (2 * lid + 1) - 1;
			int bi = offset * (2 * lid + 2) - 1;

			for (int i = 0; i < 4; i++)
				mask[4*bi+i] += mask[4*ai+i];
		}
		offset *= 2;
	}

	barrier ();
	memoryBarrierShared ();
	
	if (lid == 0)
	{
		uvec4 tmp;
		tmp.x = 0;
		tmp.y = mask[4* (n - 1)];
		tmp.z = tmp.y + mask[4*(n - 1)+1];
		tmp.w = tmp.z + mask[4*(n - 1)+2];
		sblocksum = tmp;

		for (int i = 0; i < 4; i++)
		{
			blocksum[numblocks * i + gl_WorkGroupID.x] = uvec4 (mask[4*(n - 1)+i], 0, 0, 0);
			mask[4 * (n - 1) + i] = 0;
		}
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
			
			for (int i = 0; i < 4; i++)
			{
				uint tmp = mask[4*ai+i];
				mask[4*ai+i] = mask[4*bi+i];
				mask[4*bi+i] += tmp;
			}
		}
	}
	
	barrier ();
	memoryBarrierShared ();
	
	prefixsum[2 * gid] = uvec4 (mask[4 * 2 * lid + bits1], 42, 42, 42);
	prefixsum[2 * gid + 1] = uvec4 (mask[4 * (2 * lid + 1) + bits2], 84, 84, 84);
}
