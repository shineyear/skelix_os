/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <stdio.h>
#include <stdlib.h>

static const char *prog_name = "ghex";

static void
print_hex(unsigned char *buffer, size_t s) {
	size_t i = 0;
	for (; i<s; ++i)
		fprintf(stdout, "0x%02x,", buffer[i]);
}

int
main(int argc, char *argv[]) {
	int pt = 1;

	if (argc == 1) {
		fprintf(stderr, "Usage: %s <files>\n", prog_name);
		exit(EXIT_FAILURE);
	}

	do {
		size_t s;
		char *file_name = argv[pt];
		FILE *fp = NULL;
		static unsigned char buffer[BUFSIZ];
		static const char title[] = "######";
		/* if multiple files, then print title, the file name
		   is leading by six hash mark */
		if (argc > 2)
			fprintf(stdout, "%s %s\n", title, file_name);

		fp = fopen(file_name, "r");
		if (fp == NULL) {
			fprintf(stderr, "%s: Can not open file %s\n",
					prog_name, file_name);
			if ( fclose(fp))
				fprintf(stderr, "%s: Can not close file %s\n",
						prog_name, file_name);
			continue;
		}

		while ( (s=fread(buffer, 1, BUFSIZ, fp)))
			print_hex(buffer, s);
		fprintf(stdout, "\n");

		if (ferror(fp)) {
			fprintf(stderr, "%s: Error occured when reading file %s\n",
					prog_name, file_name);
			if ( fclose(fp))
				fprintf(stderr, "%s: Can not close file %s\n",
						prog_name, file_name);
			continue;
		}
	}while (pt++ < argc-1);

	exit(0);
}
