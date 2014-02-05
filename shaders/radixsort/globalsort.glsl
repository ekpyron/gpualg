// header is included here

layout (local_size_x = BLOCKSIZE) in;

layout (std430, binding = 0) readonly buffer Data
{
	uvec4 data[];
};

layout (std430, binding = 1) readonly buffer PrefixSum
{
	uvec4 prefixsum[];
};

layout (std430, binding = 2) readonly buffer BlockSum
{
	uvec4 blocksum[];
};

layout (std430, binding = 3) writeonly buffer Result
{
	uvec4 result[];
};

uniform uint numblocks;

uniform int bitshift;

void main (void)
{
	const int gid = int (gl_GlobalInvocationID.x);
	const int lid = int (gl_LocalInvocationIndex);

	uint d = data[gid].x;
	uint bits = (d & (3 << bitshift)) >> bitshift;
	
	result[blocksum[numblocks * bits + gl_WorkGroupID.x].x + prefixsum[gid].x] = data[gid];
}
