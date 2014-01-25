#include "RadixSort.h"

RadixSort::RadixSort (void) : blocksize (256), numblocks (16)
{
	std::stringstream header;
	std::string src;

	header << "#version 430 core" << std::endl
			<< "#define BLOCKSIZE " << blocksize << std::endl
			<< "#define HALFBLOCKSIZE " << (blocksize / 2) << std::endl
			<< "#line 1" << std::endl;
	src = LoadFile ("shaders/radixsort/counting.glsl");
	if (!counting.Create (GL_COMPUTE_SHADER, header.str () + src))
	{
		throw std::runtime_error (std::string ("Cannot load radix counting shader: ") + counting.GetInfoLog ());
	}
	src = LoadFile ("shaders/radixsort/blockscan.glsl");
	if (!blockscan.Create (GL_COMPUTE_SHADER, header.str () + src))
	{
		throw std::runtime_error (std::string ("Cannot load blockscan counting shader: ") + blockscan.GetInfoLog ());
	}
	src = LoadFile ("shaders/radixsort/globalsort.glsl");
	if (!globalsort.Create (GL_COMPUTE_SHADER, header.str () + src))
	{
		throw std::runtime_error (std::string ("Cannot load blockscan counting shader: ") + globalsort.GetInfoLog ());
	}

	std::vector<uint32_t> data;

	srand (42);
	for (int i = 0; i < blocksize*numblocks; i++)
	{
		data.push_back (rand () & 3);
		std::cout << data.back () << " ";
	}
	std::cout << std::endl << std::endl;

	buffer.Data (sizeof (uint32_t) * data.size (), &data[0], GL_STATIC_DRAW);
	prefixsums.Data (sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);
	blocksums.Data (sizeof (uint32_t) * 4 * ((numblocks + blocksize - 1)%(~(blocksize-1))), NULL, GL_STATIC_DRAW);
	blocksumsums.Data (sizeof (uint32_t), NULL, GL_STATIC_DRAW);

	result.Data (sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);

	glm::uvec4 blocksumoffsets (0, numblocks, numblocks * 2, numblocks * 3);
	counting["blocksumoffsets"] = blocksumoffsets;
	globalsort["blocksumoffsets"] = blocksumoffsets;
}

RadixSort::~RadixSort (void)
{
}

void RadixSort::Run (void)
{
	buffer.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	prefixsums.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 2);
	blocksumsums.BindBase (GL_SHADER_STORAGE_BUFFER, 3);
	result.BindBase (GL_SHADER_STORAGE_BUFFER, 4);

	blocksums.ClearData (GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, NULL);
	gl::MemoryBarrier (GL_BUFFER_UPDATE_BARRIER_BIT);

	counting.Use ();
	gl::DispatchCompute (numblocks, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	blockscan.Use ();
	gl::DispatchCompute (1, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	globalsort.Use ();
	gl::DispatchCompute (numblocks, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);


	uint32_t *data = reinterpret_cast<uint32_t*> (result.Map (GL_READ_ONLY));
	for (int i = 0; i < blocksize * numblocks; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	result.Unmap ();



}
