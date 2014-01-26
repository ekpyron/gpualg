// header is included here

layout (local_size_x = BLOCKSIZE) in;

layout (std430, binding = 0) buffer Data
{
	uint data[];
};

layout (std430, binding = 1) buffer PrefixSum
{
	uint prefixsum[];
};

layout (std430, binding = 2) buffer BlockSum
{
	uint blocksum[];
};

layout (std430, binding = 3) buffer Result
{
	uint result[];
};

uniform uvec4 blocksumoffsets;


void main (void)
{
	const int gid = int (gl_GlobalInvocationID.x);
	const int lid = int (gl_LocalInvocationIndex);

	uint d = data[gid];
	uint bits = d & 3;
	
	result[blocksum[blocksumoffsets[bits] + gl_WorkGroupID.x] + prefixsum[gid]] = data[gid];
}
