// header is included here

layout (local_size_x = BLOCKSIZE) in;

layout (std430, binding = 0) buffer Data
{
	uvec4 data[];
};

layout (std430, binding = 1) readonly buffer BlockSums
{
	uvec4 blocksums[];
};

void main (void)
{
	data[gl_GlobalInvocationID.x] += blocksums[gl_WorkGroupID.x];
}
