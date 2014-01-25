#include "RadixSort.h"

RadixSort::RadixSort (void)
{
	std::string src;
	src = LoadFile ("shaders/radixsort/counting.glsl");
	if (!counting.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load radix counting shader: ") + counting.GetInfoLog ());
	}

	std::vector<uint32_t> data;

	srand (42);
	for (int i = 0; i < 256; i++)
	{
		data.push_back (rand () & 3);
		std::cout << data.back () << " ";
	}
	std::cout << std::endl << std::endl;

	buffer.Data (sizeof (uint32_t) * data.size (), &data[0], GL_STATIC_DRAW);
	prefixsums.Data (sizeof (uint32_t) * data.size (), NULL, GL_STATIC_DRAW);
	blocksums.Data (sizeof (uint32_t) * 256, NULL, GL_STATIC_DRAW);
}

RadixSort::~RadixSort (void)
{
}

void RadixSort::Run (void)
{
	counting.Use ();

	buffer.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	prefixsums.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 2);
	gl::DispatchCompute (1, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	uint32_t *data = reinterpret_cast<uint32_t*> (prefixsums.Map (GL_READ_ONLY));
	for (int i = 0; i < 256; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	prefixsums.Unmap ();

	data = reinterpret_cast<uint32_t*> (buffer.Map (GL_READ_ONLY));
	for (int i = 0; i < 256; i++)
	{
		std::cout << data[i] << " ";
	}
	std::cout << std::endl << std::endl;
	buffer.Unmap ();

}
