#version 430 core

layout (local_size_x = 256) in;

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

layout (std430, binding = 4) buffer Result
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
	
	result[blocksum[blocksumoffsets[d] + gl_WorkGroupID.x] + prefixsum[gid]] = data[gid];
}
