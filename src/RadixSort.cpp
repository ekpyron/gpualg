#include "RadixSort.h"

RadixSort::RadixSort (void)
	: blocksize (512), numblocks (4)
{
	std::stringstream header;
	std::string src;

	header << "#version 430 core" << std::endl
			<< "#define BLOCKSIZE " << blocksize << std::endl
			<< "#define HALFBLOCKSIZE " << (blocksize / 2) << std::endl
			<< "#line 1" << std::endl;
	src = LoadFile ("shaders/radixsort/counting.glsl");
	counting = LoadShaderProgram(GL_COMPUTE_SHADER, header.str () + src);
	src = LoadFile ("shaders/radixsort/blockscan.glsl");
	blockscan = LoadShaderProgram (GL_COMPUTE_SHADER, header.str () + src);
	src = LoadFile ("shaders/radixsort/globalsort.glsl");
	globalsort = LoadShaderProgram (GL_COMPUTE_SHADER, header.str () + src);
	src = LoadFile ("shaders/radixsort/addblocksum.glsl");
	addblocksum = LoadShaderProgram (GL_COMPUTE_SHADER, header.str () + src);

	std::vector<uint32_t> data;

	srand (42);
	for (int i = 0; i < blocksize * numblocks; i++)
	{
		data.push_back (rand () & 3);
		std::cout << std::hex << data[i] << " ";
	}
	std::cout << std::endl << std::endl;

	glGenBuffers (3, buffers);

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffer);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (uint32_t) * data.size (), &data[0], GL_STATIC_DRAW);
	glBindBuffer (GL_SHADER_STORAGE_BUFFER, prefixsums);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);

	uint32_t numblocksums = 4 * numblocks;
	{
		int n = ceil (log (((numblocksums + blocksize - 1) / blocksize) * blocksize) / log (blocksize));
		n++;
		blocksums.resize (n);
		glGenBuffers (n, &blocksums[0]);
	}
	for (GLuint &blocksum : blocksums)
	{
		glBindBuffer (GL_SHADER_STORAGE_BUFFER, blocksum);
		numblocksums = ((numblocksums + blocksize - 1) / blocksize) * blocksize;
		if (numblocksums < 1)
			numblocksums = 1;
		glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (uint32_t) * numblocksums, NULL, GL_STATIC_DRAW);
		numblocksums /= blocksize;
	}

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, result);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);

	glm::uvec4 blocksumoffsets (0, numblocks, numblocks * 2, numblocks * 3);
	glProgramUniform4uiv (counting, glGetUniformLocation (counting, "blocksumoffsets"), 1,
			glm::value_ptr (blocksumoffsets));
	glProgramUniform4uiv (globalsort, glGetUniformLocation (globalsort, "blocksumoffsets"), 1,
			glm::value_ptr (blocksumoffsets));
}

RadixSort::~RadixSort (void)
{
	glDeleteBuffers (blocksums.size (), &blocksums[0]);
	glDeleteBuffers (3, buffers);
	for (int i = 0; i < 4; i++)
	{
		glDeleteProgram (programs[i]);
	}
}

void RadixSort::Run (void)
{
	for (int blocksum : blocksums)
	{
		glBindBuffer (GL_SHADER_STORAGE_BUFFER, blocksum);
		glClearBufferData (GL_SHADER_STORAGE_BUFFER, GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, NULL);
	}
	glMemoryBarrier (GL_BUFFER_UPDATE_BARRIER_BIT);

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, prefixsums);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 2, blocksums.front ());
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 3, result);

	glUseProgram (counting);
	glDispatchCompute (numblocks, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram (blockscan);
	uint32_t numblocksums = (4 * numblocks) / blocksize;
	for (int i = 0; i < blocksums.size () - 1; i++)
	{
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, blocksums[i]);
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, blocksums[i + 1]);
		glDispatchCompute (numblocksums > 0 ? numblocksums : 1, 1, 1);
		numblocksums /= blocksize;
		glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);
	}

	glUseProgram (addblocksum);
	for (int i = blocksums.size () - 3; i >= 0; i--)
	{
		uint32_t numblocksums = (4 * numblocks) / blocksize;
		for (int j = 0; j < i; j++)
			numblocksums /= blocksize;
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, blocksums[i]);
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, blocksums[i + 1]);
		glDispatchCompute (numblocksums > 0 ? numblocksums : 1, 1, 1);
		glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);
	}

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, prefixsums);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 2, blocksums.front ());
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 3, result);

	glUseProgram (globalsort);
	glDispatchCompute (numblocks, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, result);
	uint32_t *data = reinterpret_cast<uint32_t*> (glMapBuffer (GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
	for (int i = 0; i < blocksize * numblocks; i++)
	{
		std::cout << std::hex << data[i] << " ";
	}
	std::cout << std::endl << std::endl;

	bool issorted = true;
	for (int i = 0; i < blocksize * numblocks - 1; i++)
	{
		if (data[i] > data[i + 1])
		{
			issorted = false;
			std::cout << "WRONG (" << i << "): " << data[i] << " > " << data[i + 1] << std::endl;
			break;
		}
	}

	if (issorted)
		std::cout << "SORTED!" << std::endl;
	glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);

}
