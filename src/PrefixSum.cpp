#include "PrefixSum.h"

PrefixSum::PrefixSum (void)
{
	std::string src;
	src = LoadFile ("shaders/prefixsum/blockscan.glsl");
	if (!blockscan.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load shader: ") + blockscan.GetInfoLog ());
	}

	src = LoadFile ("shaders/prefixsum/addblocksum.glsl");
	if (!addblocksum.Create (GL_COMPUTE_SHADER, src))
	{
		throw std::runtime_error (std::string ("Cannot load shader: ") + addblocksum.GetInfoLog ());
	}

	std::vector<float> data;

	for (int i = 0; i < 65536; i++)
		data.push_back (i);

	buffer.Data (sizeof (float) * data.size (), &data[0], GL_STATIC_DRAW);
	blocksums.Data (sizeof (float) * 256, NULL, GL_STATIC_DRAW);
	unused.Data (sizeof (float), NULL, GL_STATIC_DRAW);
}

PrefixSum::~PrefixSum (void)
{
}

void PrefixSum::Run (void)
{
	blockscan.Use ();

	buffer.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	gl::DispatchCompute (256, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	unused.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	gl::DispatchCompute (256, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	buffer.BindBase (GL_SHADER_STORAGE_BUFFER, 0);
	blocksums.BindBase (GL_SHADER_STORAGE_BUFFER, 1);
	addblocksum.Use ();
	gl::DispatchCompute (256, 1, 1);
	gl::MemoryBarrier (GL_SHADER_STORAGE_BARRIER_BIT);

	float *data = reinterpret_cast<float*> (buffer.Map (GL_READ_ONLY));
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
	buffer.Unmap ();

}
