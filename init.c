/*
 * initializate various things
 *
 * @(#)init.c	9.0	(rdk)	 7/17/84
 *
 * Super-Rogue
 * Copyright (C) 1984 Robert D. Kindelberger
 * All rights reserved.
 *
 * Based on "Rogue: Exploring the Dungeons of Doom"
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

char *rainbow[NCOLORS] = {
	"Red",		"Blue",		"Green",	"Yellow",
	"Black",	"Brown",	"Orange",	"Pink",
	"Purple",	"Grey",		"White",	"Silver",
	"Gold",		"Violet",	"Clear",	"Vermilion",
	"Ecru",		"Turquoise","Magenta",	"Amber",
	"Topaz",	"Plaid",	"Tan",		"Tangerine",
	"Aquamarine", "Scarlet","Khaki",	"Crimson",
	"Indigo",	"Beige",	"Lavender",	"Saffron",
};

char *sylls[NSYLS] = {
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
	"ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
	"blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
	"dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
	"glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
	"ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
	"man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
	"nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
	"ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
	"pot","prok","re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
	"sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
	"sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
	"tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
	"wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
	"zant", "zap", "zeb", "zim", "zok", "zon", "zum",
};

char *stones[] = {
	"Agate",		"Alexandrite",	"Amethyst",
	"Azurite",		"Carnelian",	"Chrysoberyl",
	"Chrysoprase",	"Citrine",		"Diamond",
	"Emerald",		"Garnet",		"Hematite",
	"Jacinth",		"Jade",			"Kryptonite",
	"Lapus lazuli",	"Malachite",	"Moonstone",
	"Obsidian",		"Olivine",		"Onyx",
	"Opal",			"Pearl",		"Peridot",
	"Quartz",		"Rhodochrosite","Ruby",
	"Sapphire",		"Sardonyx",		"Serpintine",
	"Spinel",		"Tiger eye",	"Topaz",
	"Tourmaline",	"Turquoise",
};

char *wood[NWOOD] = {
	"Avocado wood",	"Balsa",	"Banyan",		"Birch",
	"Cedar",		"Cherry",	"Cinnibar",		"Dogwood",
	"Driftwood",	"Ebony",	"Eucalyptus",	"Hemlock",
	"Ironwood",		"Mahogany",	"Manzanita",	"Maple",
	"Oak",			"Pine",		"Redwood",		"Rosewood",
	"Teak",			"Walnut",	"Zebra wood", 	"Persimmon wood",
};

char *metal[NMETAL] = {
	"Aluminium",	"Bone",		"Brass",	"Bronze",
	"Copper",		"Chromium",	"Iron",		"Lead",
	"Magnesium",	"Pewter",	"Platinum",	"Steel",
	"Tin",			"Titanium",	"Zinc",
};

/*
 * init_everything:
 *	Set up all important stuff.
 */
init_everything()
{
	init_player();			/* Roll up the rogue */
	init_things();			/* Set up probabilities */
	init_names();			/* Set up names of scrolls */
	init_colors();			/* Set up colors of potions */
	init_stones();			/* Set up stones in rings */
	init_materials();		/* Set up materials of wands */
}

/*
 * init_things:
 *	Initialize the probabilities for types of things
 */
init_things()
{
	struct magic_item *mi;
	
	/*
	 * init general things
	 */
	for (mi = &things[1]; mi < &things[NUMTHINGS]; mi++)
		mi->mi_prob += (mi-1)->mi_prob;
	badcheck("things", things);
	/*
	 * init armor things
	 */
	for (mi = &a_magic[1]; mi < &a_magic[MAXARMORS]; mi++)
		mi->mi_prob += (mi-1)->mi_prob;
	badcheck("armor", a_magic);
	/*
	 * init weapon stuff
	 */
	for (mi = &w_magic[1]; mi < &w_magic[MAXWEAPONS]; mi++)
		mi->mi_prob += (mi-1)->mi_prob;
	badcheck("weapon", w_magic);
}


/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */
init_colors()
{
	reg int i, j;
	reg char *str;
	bool used[NCOLORS];

	for (i = 0; i < NCOLORS; i++)
		used[i] = FALSE;
	for (i = 0; i < MAXPOTIONS; i++) {
		do {
			j = rnd(NCOLORS);
		} until (!used[j]);
		used[j] = TRUE;
		p_colors[i] = rainbow[j];
		p_know[i] = FALSE;
		p_guess[i] = NULL;
		if (i > 0)
			p_magic[i].mi_prob += p_magic[i-1].mi_prob;
	}
	badcheck("potions", p_magic);
}


/*
 * init_names:
 *	Generate the names of the various scrolls
 */
init_names()
{
	reg int nsyl;
	reg char *cp, *sp;
	reg int i, nwords;

	for (i = 0; i < MAXSCROLLS; i++) {
		cp = prbuf;
		nwords = rnd(3)+1;
		while(nwords--)	{
			nsyl = rnd(3)+2;
			while(nsyl--) {
				sp = sylls[rnd(NSYLS)];
				while(*sp)
					*cp++ = *sp++;
			}
			*cp++ = ' ';
		}
		*--cp = '\0';
		s_names[i] = new(strlen(prbuf)+1);
		s_know[i] = FALSE;
		s_guess[i] = NULL;
		strcpy(s_names[i], prbuf);
		if (i > 0)
			s_magic[i].mi_prob += s_magic[i-1].mi_prob;
	}
	badcheck("scrolls", s_magic);
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */

init_stones()
{
	reg int i, j;
	reg char *str;
	bool used[NSTONES];

	for (i = 0; i < NSTONES; i++)
		used[i] = FALSE;

	for (i = 0; i < MAXRINGS; i++) {
		do {
			j = rnd(NSTONES);
		} until (!used[j]);
		used[j] = TRUE;
		r_stones[i] = stones[j];
		r_know[i] = FALSE;
		r_guess[i] = NULL;
		if (i > 0)
			r_magic[i].mi_prob += r_magic[i-1].mi_prob;
	}
	badcheck("rings", r_magic);
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */

init_materials()
{
	int i, j;
	char *str;
	struct rod *rd;
	bool metused[NMETAL], woodused[NWOOD];

	for (i = 0; i < NWOOD; i++)
		woodused[i] = FALSE;
	for (i = 0; i < NMETAL; i++)
		metused[i] = FALSE;

	for (i = 0; i < MAXSTICKS; i++) {
		rd = &ws_stuff[i];
		for (;;)  {
			if (rnd(100) > 50) {
				j = rnd(NMETAL);
				if (!metused[j]) {
					str = metal[j];
					rd->ws_type = "wand";
					rd->ws_vol = V_WS_WAND;
					rd->ws_wght = W_WS_WAND;
					metused[j] = TRUE;
					break;
				}
			}
			else {
				j = rnd(NWOOD);
				if (!woodused[j]) {
					str = wood[j];
					rd->ws_type = "staff";
					rd->ws_vol = V_WS_STAFF;
					rd->ws_wght = W_WS_WAND;
					woodused[j] = TRUE;
					break;
				}
			}
		} 
		ws_stuff[i].ws_made = str;
		ws_know[i] = FALSE;
		ws_guess[i] = NULL;
		if (i > 0)
			ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
	}
	badcheck("sticks", ws_magic);
}

badcheck(name, magic)
char *name;
struct magic_item *magic;
{
	struct magic_item *mg;

	for (mg = magic; mg->mi_name != NULL; mg++)
		;
	if ((mg - 1)->mi_prob == 1000)
		return;
	printf("\nBad percentages for %s:\n", name);
	for (mg = magic; mg->mi_name != NULL; mg++)
		printf("%4d%% %s\n", mg->mi_prob, mg->mi_name);
	printf("%s", retstr);
	fflush(stdout);
	while (getchar() != '\n')
		continue;
}


/*
 * init_player:
 *	roll up the rogue
 */

init_player()
{
	player.t_nomove = 0;
	player.t_nocmd = 0;
	him = &player.t_stats;
	him->s_lvl = 1;
	him->s_exp = 0L;
	him->s_maxhp = him->s_hpt = pinit();		/* hit points */
	him->s_re.a_str = pinit();		/* strength */
	him->s_re.a_dex = pinit();		/* dexterity */
	him->s_re.a_wis = pinit();		/* wisdom */
	him->s_re.a_con = pinit();		/* constitution */
	him->s_ef = him->s_re;			/* effective = real */
	strcpy(him->s_dmg, "1d4");
	him->s_arm = NORMAC;
	him->s_carry = totalenc();
	him->s_pack = 0;
	pack = NULL;				/* empty pack so far */
	max_stats = *him;
}


/*
 * pinit:
 *	Returns the best 3 of 4 on a 6-sided die
 */
pinit()
{
	int best[4];
	reg int i, min, minind, dicetot;

	for (i = 0 ; i < 4 ; i++)
		best[i] = roll(1,6);	/* populate array */
	min = best[0];				/* assume that 1st entry */
	minind = 0;					/* is the lowest */
	for (i = 1 ; i < 4 ; i++) {	/* find the lowest */
		if (best[i] < min) {	/* if < minimum then update */
			min = best[i];
			minind = i;			/* point to lowest value */
		}
	}
	dicetot = 0;				/* start with nothing */
	for (i = 0 ; i < 4 ; i++) {
		if (i != minind)		/* if not minimum, then add it */
			dicetot += best[i];
	}
	return(dicetot);
}
