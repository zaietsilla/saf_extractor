#define _CRT_SECURE_NO_WARNINGS // strerror
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <direct.h>
#include <errno.h>
#include <string.h>
#include <conio.h>

#pragma warning(disable: 4244)

typedef struct
{
	int name_length;
	char* name;
	uint64_t offset;
	uint64_t length;
} File;

void sah_read(char* path, FILE* stream);
void saf_read(const char* path, const File* file);

int main()
{
	FILE* stream;

	if (!fopen_s(&stream, "data.sah", "rb"))
	{
		// ignore the file header
		fseek(stream, 51, SEEK_SET);

		int name_length;
		fread(&name_length, 4, 1, stream);

		if (name_length == 1)
		{
			// read the old format
			fseek(stream, 1, SEEK_CUR);
		}
		else
		{
			// read the new format
			fseek(stream, 5, SEEK_CUR);
		}

		_mkdir("Data");

		// read the file contents
		sah_read("Data", stream);
	}
	else
	{
		fprintf(stderr, "%s %s\n", strerror(errno), "data.sah");
		_getch();
		return -1;
	}

	fclose(stream);

	printf("Press any key to exit...\n");
	_getch();

	_CrtDumpMemoryLeaks();
	return 0;
}

void sah_read(char* path, FILE* stream)
{
	printf("%s\n", path);

	int file_count;
	fread(&file_count, 4, 1, stream);
	
	// read and process the files
	for (int i = 0; i < file_count; ++i)
	{
		File file;

		fread(&file.name_length, 4, 1, stream);

		file.name = malloc(file.name_length);
		fread(file.name, file.name_length, 1, stream);

		// replace invalid chars
		for (int j = 0; j < file.name_length; ++j)
		{
			if (file.name[j] == '?')
			{
				file.name[j] = '0';
			}
		}

		fread(&file.offset, 8, 1, stream);
		fread(&file.length, 8, 1, stream);
		
		// process the file data
		saf_read(path, &file);

		free(file.name);
	}

	int folder_count;
	fread(&folder_count, 4, 1, stream); 

	char* folder = malloc(0);

	// read the and process the folders
	for (int i = 0; i < folder_count; ++i)
	{
		int name_length;
		fread(&name_length, 4, 1, stream);

		char* name = malloc(name_length);
		fread(name, name_length, 1, stream);

		// write the path string
		int length = strlen(path) + name_length + 2;
		folder = realloc(folder, length);
		snprintf(folder, length, "%s\\%s", path, name);

		free(name);

		// replace invalid chars
		for (int j = 0; j < length; ++j)
		{
			if (folder[j] == '?')
			{
				folder[j] = '0';
			}
		}

		_mkdir(folder);

		// read the file contents
		sah_read(folder, stream);
	}

	free(folder);
}

void saf_read(const char* path, const File* file)
{
	FILE* stream;

	// warning C4244
	void* buffer = malloc(file->length);

	if (!fopen_s(&stream, "data.saf", "rb"))
	{
		// read the file data from the archive
		_fseeki64(stream, file->offset, SEEK_SET);

		// warning C4244
		fread(buffer, file->length, 1, stream);
	}
	else
	{
		fprintf(stderr, "%s %s\n", strerror(errno), "data.saf");
		_getch();
		_exit(-1);
	}
	
	// write the path string
	int length = strlen(path) + file->name_length + 2;
	char* filename = malloc(length);
	snprintf(filename, length, "%s\\%s", path, file->name);

	if (!freopen_s(&stream, filename, "wb", stream))
	{
		// warning C4244
		fwrite(buffer, file->length, 1, stream);

		free(filename);
		free(buffer);
		fclose(stream);
	}
	else
	{
		fprintf(stderr, "%s %s\n", strerror(errno), filename);
		_getch();
		_exit(-1);
	}
}
