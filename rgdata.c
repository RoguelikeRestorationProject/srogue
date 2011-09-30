/*
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "global.c"

main(argc,argv)
char **argv;
int argc;
{
	char *ptr;
	int i, j, k;
	struct magic_item *mi;
	struct init_weps *wp;
	struct init_armor *ar;
	struct monster *mo;
	FILE *fo;

	/*
	 * write to desired output file
	 */
	if (argc > 1) {
		fo = fopen(argv[1], "w");
		if (fo == NULL) {
		printf("%s: %s not writable\n",argv[0],argv[1]);
		exit(1);
		}
	}
	else
		fo = stdout;

	/*
	 * print total chances for armor, weapons, food, scrolls, etc
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t	ITEM GENERAL INFO\n\n\n");
	fprintf(fo,"NAME\t\tCHANCE\t\tWEIGHT\n\n");
	for (mi = &things[0]; mi < &things[NUMTHINGS]; mi++) {
		fprintf(fo,"%s\t\t",mi->mi_name);
		i = mi->mi_prob / 10;
		j = mi->mi_prob % 10;
		fprintf(fo,"%2d.%1d %%\t\t",i,j);
		i = mi->mi_wght / 10;
		j = mi->mi_wght % 10;
		if (i == 0 && j == 0)
			fprintf(fo,"%3s\n","*");
		else
			fprintf(fo,"%3d.%1d lbs\n",i,j);
	}
	fprintf(fo,"\n\n\n\n\n\n\nNOTES - * means that weight depends on which one of that item type\n");
	fprintf(fo,"	  - All items weigh 20%% more when cursed\n");

	/*
	 * print stuff about potions
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t  POTION INFO\n\n\n");
	fprintf(fo,"NAME\t\t\t\tCHANCE\t\tWORTH\n\n");
	for (mi = &p_magic[0]; mi < &p_magic[MAXPOTIONS]; mi++) {
		fprintf(fo,"%s\t",mi->mi_name);
		k = strlen(mi->mi_name);
		if (k < 8)
			ptr = "\t\t\t";
		else if (k >= 16)
			ptr = "\t";
		else
			ptr = "\t\t";
		fprintf(fo,"%s", ptr);
		i = mi->mi_prob / 10;
		j = mi->mi_prob % 10;
		fprintf(fo,"%2d.%1d %%\t\t",i,j);
		fprintf(fo,"%3d\n",mi->mi_worth);
	}
	fprintf(fo,"\n\n\n\n\nNOTE - All potions weigh 0.5 lbs\n");
	

	/*
	 * print stuff about scrolls
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t  SCROLL INFO\n\n\n");
	fprintf(fo,"NAME\t\t\t\tCHANCE\t\tWORTH\n\n");
	for (mi = &s_magic[0]; mi < &s_magic[MAXSCROLLS]; mi++) {
		fprintf(fo,"%s\t",mi->mi_name);
		k = strlen(mi->mi_name);
		if (k < 8)
			ptr = "\t\t\t";
		else if (k >= 16)
			ptr = "\t";
		else
			ptr = "\t\t";
		fprintf(fo,"%s", ptr);
		i = mi->mi_prob / 10;
		j = mi->mi_prob % 10;
		fprintf(fo,"%2d.%1d %%\t\t",i,j);
		fprintf(fo,"%3d\n",mi->mi_worth);
	}
	fprintf(fo,"\n\n\n\n\nNOTE - All scrolls weigh 3.0 lbs\n");


	/*
	 * print stuff about rings
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t  RING INFO\n\n\n");
	fprintf(fo,"NAME\t\t\t\tCHANCE\t\tWORTH\n\n");
	for (mi = &r_magic[0]; mi < &r_magic[MAXRINGS]; mi++) {
		fprintf(fo,"%s\t",mi->mi_name);
		k = strlen(mi->mi_name);
		if (k < 8)
			ptr = "\t\t\t";
		else if (k >= 16)
			ptr = "\t";
		else
			ptr = "\t\t";
		fprintf(fo,"%s", ptr);
		i = mi->mi_prob / 10;
		j = mi->mi_prob % 10;
		fprintf(fo,"%2d.%1d %%\t\t",i,j);
		fprintf(fo,"%3d\n",mi->mi_worth);
	}
	fprintf(fo,"\n\n\n\n\nNOTE - All rings weigh 0.5 lbs\n");


	/*
	 * print stuff about sticks
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t  STAFF/WAND INFO\n\n\n");
	fprintf(fo,"NAME\t\t\t\tCHANCE\t\tWORTH\n\n");
	for (mi = &ws_magic[0]; mi < &ws_magic[MAXSTICKS]; mi++) {
		fprintf(fo,"%s\t",mi->mi_name);
		k = strlen(mi->mi_name);
		if (k < 8)
			ptr = "\t\t\t";
		else if (k >= 16)
			ptr = "\t";
		else
			ptr = "\t\t";
		fprintf(fo,"%s", ptr);
		i = mi->mi_prob / 10;
		j = mi->mi_prob % 10;
		fprintf(fo,"%2d.%1d %%\t\t",i,j);
		fprintf(fo,"%3d\n",mi->mi_worth);
	}
	fprintf(fo,"\n\n\n\n\nNOTES - All wands weigh 6.0 lbs\n");
	fprintf(fo,"	  - All staffs weigh 10.0 lbs\n");
	fprintf(fo,"	  - Wands contain from 4 to 8 charges\n");
	fprintf(fo,"	  - Staffs contain from 5 to 12 charges\n");
	fprintf(fo,"	  - Sticks of light have an additional 7 to 15 charges\n");


	/*
	 * print armor info
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t\t\tARMOR INFO\n\n\n");
	fprintf(fo,"NAME\t\t\t\tAC\tCHANCE\t\tWORTH\t\tWEIGHT\n\n");
	for (ar = &armors[0]; ar < &armors[MAXARMORS]; ar++) {
		fprintf(fo,"%s\t",ar->a_name);
		k = strlen(ar->a_name);
		if (k < 8)
			ptr = "\t\t\t";
		else if (k >= 16)
			ptr = "\t";
		else
			ptr = "\t\t";
		fprintf(fo,"%s", ptr);
		fprintf(fo,"%2d\t",ar->a_class);
		fprintf(fo,"%2d %%\t\t",ar->a_prob);
		fprintf(fo,"%3d\t\t",ar->a_worth);
		fprintf(fo,"%2d lbs\n",ar->a_wght / 10);
	}
	fprintf(fo,"\n\n\n\n\nNOTE - All armor becomes 50%% lighter when blessed\n");


	/*
	 * print stuff about weapons
	 */
	fprintf(fo,"\n\n\n\n\n\n");
	fprintf(fo,"\t\t\t\t\tWEAPON INFO\n\n\n");
	fprintf(fo,
	   "NAME\t\t\tHIT DAMAGE\tHURL DAMAGE\tWORTH\t\tWEIGHT\n\n");
	for (wp = &weaps[0]; wp < &weaps[MAXWEAPONS]; wp++) {
		fprintf(fo,"%s\t",wp->w_name);
		k = strlen(wp->w_name);
		if (k < 8)
			ptr = "\t\t";
		else if (k >= 16)
			ptr = "";
		else
			ptr = "\t";
		fprintf(fo,"%s", ptr);
		ptr = wp->w_dam;
		i = *ptr - '0';
		j = 0;
		ptr += 2;
		while (*ptr != NULL) {
			j = j * 10 + (*ptr - '0');
			++ptr;
		}
		j *= i;
		fprintf(fo,"  %d to %d\t",i,j);
		ptr = wp->w_hrl;
		i = *ptr - '0';
		j = 0;
		ptr += 2;
		while (*ptr != NULL) {
			j = j * 10 + (*ptr - '0');
			++ptr;
		}
		j *= i;
		fprintf(fo,"  %d to %d\t",i,j);
		fprintf(fo,"%4d\t\t",wp->w_worth);
		i = wp->w_wght / 10;
		j = wp->w_wght % 10;
		fprintf(fo,"%2d.%1d lbs\n",i,j);
	}
	fprintf(fo,"\n\n\n\n\nNOTE - All weapons become 50%% lighter when blessed\n");


	/*
	 * print stuff about the monsters
	 */
}
