#include "PrefixSum.h"

PrefixSum::PrefixSum (void)
{
	std::string src;
	src = LoadFile ("shaders/prefixsum/blockscan.glsl");
	blockscan = LoadShaderProgram (GL_COMPUTE_SHADER, src);

	src = LoadFile ("shaders/prefixsum/addblocksum.glsl");
	addblocksum = LoadShaderProgram (GL_COMPUTE_SHADER, src);

	std::vector<float> data;

	for (int i = 0; i < 65536; i++)
		data.push_back (i);

	glGenBuffers (3, buffers);
	glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffer);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (float) * data.size (), &data[0], GL_STATIC_DRAW);
	glBindBuffer (GL_SHADER_STORAGE_BUFFER, blocksums);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (float) * 256, NULL, GL_STATIC_DRAW);
	glBindBuffer (GL_SHADER_STORAGE_BUFFER, unused);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (float), NULL, GL_STATIC_DRAW);
}

PrefixSum::~PrefixSum (void)
{
	glDeleteBuffers (3, buffers);
	for (int i = 0; i < 2; i++)
		glDeleteProgram (programs[i]);
}

void PrefixSum::Run (void)
{
	glUseProgram (blockscan);

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, blocksums);
	glDispatchCompute (256, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, blocksums);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, unused);
	glDispatchCompute (256, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, blocksums);
	glUseProgram (addblocksum);
	glDispatchCompute (256, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffer);
	float *data = reinterpret_cast<float*> (glMapBuffer (GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			std::cout << std::setfill ('0') << std::setw (10) << uint32_t (data[i*256 + j]);
			if (j < 255)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
}
