/*
 * Change the version number of rogue
 *
 * The version must be in the file in the format of:
 *
 * " * @(#)filename\tVERSION\t ..."
 *
 * Where VERSION is a 3 character string, i.e., "8.2"
 *
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdio.h>
#include <time.h>

long clock;
struct tm *tp;
char who[100];

char *strrchr(), *strchr(), *fgets();

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	register char *ptr;
	char ts[30];
	FILE *fp;

	strcpy(who, argv[0]);
	if (argc < 3) {
		fprintf(stderr,"Usage: %s VERSION c_files\n", who);
		exit(1);
	}
	if (strlen(argv[1]) != 3) {
		fprintf(stderr,"%s: VERSION must be length 3\n", who);
		exit(1);
	}
	time(&clock);
	tp = localtime(&clock);
	sprintf(ts,"%2d/%2d/%2d",tp->tm_mon + 1,tp->tm_mday,tp->tm_year);
	for (i = 2; i < argc; i++) {
		ptr = strrchr(argv[i], '.');
		/*
		 * make sure that files end in ".c" or ".h"
		 */
		if (ptr != NULL) {
			++ptr;
			if (*ptr == 'c' || *ptr == 'h')
				updvers(argv[1], argv[i]);
		}
	}
	/*
	 * now install new "version.c" file
	 */
	fp = fopen("vers.c", "w");
	if (fp == NULL) {
		fprintf(stderr,"%s: cant write version.c file\n",who);
		exit(1);
	}
	fprintf(fp, "/*\n * version number.\n */\n");
	fprintf(fp, "char version[] = ");
	fprintf(fp, "%c@(#)vers.c\t%3s\t(rdk)\t%s%c;\n", '"',
		argv[1], ts, '"');
	fprintf(fp, "char *release = \"%s (%s)\";\n", argv[1],ts);
	fclose(fp);
	exit(0);
}

#define LINESIZ	132

updvers(vers, fname)
char *fname;
char *vers;
{
	register FILE *fp;
	register char *ptr, *c;
	char line[LINESIZ];

	if ((fp = fopen(fname, "r+")) == NULL) {
		fprintf(stderr,"%s: Not able to update %s\n", who, fname);
		return;
	}
	while ((c = fgets(line, LINESIZ, fp)) != NULL) {
		if (line[1] == '*' && line[3] == '@' && line[5] == '#') {
			ptr = strchr(line, '\t');
			if (ptr != NULL) {
				fseek(fp, -strlen(line), 1);
				sprintf(ptr, "\t%3s\t(rdk)\t%2d/%2d/%2d\n", vers,
					tp->tm_mon + 1, tp->tm_mday, tp->tm_year);
				fprintf(fp, "%s", line);
				break;
			}
		}
	}
	fclose(fp);
}
