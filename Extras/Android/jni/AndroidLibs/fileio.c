#include <stdio.h>
#include <stdlib.h>

// missing from bionic libc
FILE* fopen64(const char * filename, const char * mode)
{
	return fopen(filename, mode);
}

int fseeko64(FILE *stream, off64_t offset, int whence)
{
	return fseek( stream, (long)(offset & 0xFFFFFFFF), whence );
}

off64_t ftello64(FILE *stream)
{
	return ftell( stream );
}
