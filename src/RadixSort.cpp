#include "RadixSort.h"
#include <random>

RadixSort::RadixSort (void)
	: blocksize (256), numblocks (32)
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

	srand (42);
	for (int i = 0; i < blocksize * numblocks; i++)
	{
		data.push_back (glm::uvec4 (i, rand (), 0, 0));
	}
	std::random_device rd;
	std::mt19937 g (rd());
	std::shuffle (data.begin (), data.end (), g);

	glGenBuffers (3, buffers);

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffer);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (glm::uvec4) * data.size (), &data[0], GL_DYNAMIC_DRAW);
	glBindBuffer (GL_SHADER_STORAGE_BUFFER, prefixsums);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (uint32_t) * 4 * data.size (), NULL, GL_DYNAMIC_DRAW);

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
		glBufferData (GL_SHADER_STORAGE_BUFFER, 4 * sizeof (uint32_t) * numblocksums, NULL, GL_DYNAMIC_DRAW);
		numblocksums /= blocksize;
	}

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, result);
	glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof (glm::uvec4) * data.size (), NULL, GL_DYNAMIC_DRAW);

	glProgramUniform1ui (counting, glGetUniformLocation (counting, "numblocks"), numblocks);
	glProgramUniform1ui (globalsort, glGetUniformLocation (globalsort, "numblocks"), numblocks);

	counting_bitshift = glGetUniformLocation (counting, "bitshift");
	globalsort_bitshift = glGetUniformLocation (globalsort, "bitshift");

	for (int blocksum : blocksums)
	{
		glBindBuffer (GL_SHADER_STORAGE_BUFFER, blocksum);
		glClearBufferData (GL_SHADER_STORAGE_BUFFER, GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, NULL);
	}
	glMemoryBarrier (GL_BUFFER_UPDATE_BARRIER_BIT);

	glGenQueries (1, &query);
}

RadixSort::~RadixSort (void)
{
	glDeleteQueries (1, &query);

	glDeleteBuffers (blocksums.size (), &blocksums[0]);
	glDeleteBuffers (3, buffers);
	for (int i = 0; i < 4; i++)
	{
		glDeleteProgram (programs[i]);
	}
}

void RadixSort::Run (void)
{
	glBeginQuery (GL_TIME_ELAPSED, query);
	for (int i = 0; i < 16; i++)
	{
		SortBits (2 * i);
		std::swap (result, buffer);
	}
	glEndQuery (GL_TIME_ELAPSED);
	std::swap (result, buffer);

	GLint available = 0;
	while (!available) {
		glGetQueryObjectiv (query, GL_QUERY_RESULT_AVAILABLE, &available);
	}
	GLuint64 timeElapsed = 0;
	glGetQueryObjectui64v (query, GL_QUERY_RESULT, &timeElapsed);

	Check ();

	std::cout << "TIME ELAPSED: " << double (timeElapsed) / 1000000000 << " Seconds" << std::endl;
}

void RadixSort::Check (void)
{
	std::sort (data.begin (), data.end (),
			[] (const glm::uvec4 &a, const glm::uvec4 &b) {
		return a.x < b.x;
	});

	glBindBuffer (GL_SHADER_STORAGE_BUFFER, result);
	glm::uvec4 *bufferdata = reinterpret_cast<glm::uvec4*> (glMapBuffer (GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));

	bool issorted = true;
	for (int i = 0; i < blocksize * numblocks; i++)
	{
		if (data[i] != bufferdata[i])
		{
			issorted = false;
			std::cout << "WRONG (" << i << "): " << bufferdata[i].x << " > " << bufferdata[i + 1].x << std::endl;
			break;
		}
	}

	if (issorted)
		std::cout << "SORTED!" << std::endl;
	glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
}

uint32_t intpow (uint32_t x, uint32_t y)
{
	uint32_t r = 1;
	while (y)
	{
		if (y & 1)
			r *= x;
		y >>= 1;
		x *= x;
	}
	return r;
}

void RadixSort::SortBits (int bits)
{
	glUseProgram (counting);
	glProgramUniform1i (counting, counting_bitshift, bits);
	glProgramUniform1i (globalsort, globalsort_bitshift, bits);

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, prefixsums);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 2, blocksums.front ());
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 3, result);

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
		uint32_t numblocksums = (4 * numblocks) / intpow (blocksize, i + 1);
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, blocksums[i]);
		glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, blocksums[i + 1]);
		glDispatchCompute (numblocksums > 0 ? numblocksums : 1, 1, 1);
		glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);
	}

	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffer);
	glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, prefixsums);

	glUseProgram (globalsort);
	glDispatchCompute (numblocks, 1, 1);
	glMemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);
}
