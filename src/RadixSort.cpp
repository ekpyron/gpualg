#include "RadixSort.h"

RadixSort::RadixSort (void)
{
	std::string src;
	src = LoadFile ("shaders/radixsort/counting.glsl");
	if (!counting.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load radix counting shader: ") + counting.GetInfoLog ());
	}
	src = LoadFile ("shaders/radixsort/blockscan.glsl");
	if (!blockscan.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load blockscan counting shader: ") + blockscan.GetInfoLog ());
	}
	src = LoadFile ("shaders/radixsort/globalsort.glsl");
	if (!globalsort.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load blockscan counting shader: ") + globalsort.GetInfoLog ());
	}

	std::vector<uint32_t> data;

	srand (42);
	for (int i = 0; i < 512; i++)
	{
		data.push_back (rand () & 3);
		std::cout << data.back () << " ";
	}
	std::cout << std::endl << std::endl;

	buffer.Data (sizeof (uint32_t) * data.size (), &data[0], GL_STATIC_DRAW);
	prefixsums.Data (sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);
	blocksums.Data (sizeof (uint32_t) * 4 * 256, NULL, GL_STATIC_DRAW);
	blocksumsums.Data (sizeof (uint32_t), NULL, GL_STATIC_DRAW);

	result.Data (sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);

	glm::uvec4 blocksumoffsets (0, 16, 32, 48);
	counting["blocksumoffsets"] = blocksumoffsets;
	globalsort["blocksumoffsets"] = blocksumoffsets;
}

RadixSort::~RadixSort (void)
{
}

void RadixSort::Run (void)
{
	{
		uint32_t data[] = {4200, 4200, 4200, 4200};
		result.ClearData (GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, data);
	}

	buffer.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	prefixsums.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 2);
	blocksumsums.BindBase (GL_SHADER_STORAGE_BUFFER, 3);
	result.BindBase (GL_SHADER_STORAGE_BUFFER, 4);

	blocksums.ClearData (GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, NULL);
	gl::MemoryBarrier (GL_BUFFER_UPDATE_BARRIER_BIT);

	counting.Use ();
	gl::DispatchCompute (2, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	blockscan.Use ();
	gl::DispatchCompute (1, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	globalsort.Use ();
	gl::DispatchCompute (2, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);


	uint32_t *data = reinterpret_cast<uint32_t*> (prefixsums.Map (GL_READ_ONLY));
	for (int i = 0; i < 512; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	prefixsums.Unmap ();

	data = reinterpret_cast<uint32_t*> (buffer.Map (GL_READ_ONLY));
	for (int i = 0; i < 512; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	buffer.Unmap ();

	data = reinterpret_cast<uint32_t*> (blocksums.Map (GL_READ_ONLY));
	for (int i = 0; i < 256; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	blocksums.Unmap ();

	data = reinterpret_cast<uint32_t*> (blocksumsums.Map (GL_READ_ONLY));
	std::cout << data[0] << std::endl << std::endl;
	blocksumsums.Unmap ();


	data = reinterpret_cast<uint32_t*> (result.Map (GL_READ_ONLY));
	for (int i = 0; i < 512; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	result.Unmap ();



}
