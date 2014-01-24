#version 430 core

layout (local_size_x = 256) in;

layout (std430, binding = 0) buffer Data
{
	float data[];
};

layout (std430, binding = 1) buffer BlockSums
{
	float blocksums[];
};

void main (void)
{
	int gid = int (gl_GlobalInvocationID.x);
	data[gl_GlobalInvocationID.x] += blocksums[gl_WorkGroupID.x];
}
