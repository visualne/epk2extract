#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "mfile.h"
#include "mediatek.h"
#include "util.h"

//lzhs
//#include "lzhs/lzhs.h"

//boot and tzfw
#include <elf.h>

void extract_mtk_1bl(MFILE *in, const char *outname) {
	MFILE *out = mfopen(outname, "w+");
	if (out == NULL){
		err_exit("Can't open file %s for writing (%s)\n", outname, strerror(errno));
	}

	mfile_map(out, MTK_PBL_SIZE);
	memcpy(mdata(out, uint8_t), mdata(in, uint8_t), MTK_PBL_SIZE);

	mclose(out);
	mclose(in);
}

void split_mtk_tz(MFILE *tz, const char *destdir) {
	int n;
	size_t tz_size;

	char *dest;
	asprintf(&dest, "%s/env.o", destdir);

	MFILE *out = mfopen(dest, "w+");
	if (out == NULL)
		err_exit("Can't open file %s for writing\n", dest);

	tz_size = msize(tz) - MTK_ENV_SIZE;
	printf("Extracting env.o... (%zu bytes)\n", MTK_ENV_SIZE);

	uint8_t *indata, *outdata;
	indata = mdata(tz, uint8_t);
	outdata = mdata(out, uint8_t);

	mfile_map(out, MTK_ENV_SIZE);
	memcpy(out, tz, MTK_ENV_SIZE);

	free(dest);
	mclose(out);

	asprintf(&dest, "%s/tz.bin", destdir);

	out = mfopen(dest, "w+");
	if (out == NULL)
		err_exit("Can't open file %s for writing\n", dest);

	printf("Extracting tz.bin... (%zu bytes)\n", tz_size);
	memcpy(out, tz + MTK_ENV_SIZE, tz_size);

	free(dest);
	mclose(out);

	mclose(tz);
}

MFILE *is_mtk_boot(const char *filename) {
	MFILE *file = mopen(filename, O_RDONLY);
	uint8_t *data = mdata(file, uint8_t);
	if (file == NULL) {
		err_exit("Can't open file %s\n", filename);
	}
	if (
		(msize(file) < MTK_PBL_SIZE) ||
		(strncmp(data + 0x100, MTK_PBL_MAGIC, strlen(MTK_PBL_MAGIC)) != 0)
	){
		mclose(file);
		return NULL;
	}
	
	printf("Found valid PBL magic: "MTK_PBL_MAGIC"\n");
	return file;
}

int is_elf_mem(Elf32_Ehdr * header) {
	if (!memcmp(&header->e_ident, ELFMAG, 4))
		return 1;
	return 0;
}

MFILE *is_elf(const char *filename) {
	MFILE *file = mopen(filename, O_RDONLY);
	if (file == NULL) {
		err_exit("Can't open file %s\n", filename);
	}
	
	Elf32_Ehdr *elfHdr = mdata(file, Elf32_Ehdr);
	if (!memcmp(&(elfHdr->e_ident), ELFMAG, 4))
		return file;
	
	mclose(file);
	return NULL;
}