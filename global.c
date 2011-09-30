/*
 * global variable declaration
 *
 * @(#)global.c	9.0	(rdk)	 7/17/84
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

#include "rogue.h"

struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room *oldrp;					/* Roomin(&oldpos) */
struct linked_list *mlist = NULL;	/* monsters on this level */
struct thing player;				/* The rogue */
struct stats max_stats;				/* The maximum for the player */
struct linked_list *lvl_obj = NULL;	/* objects on this level */
struct object *cur_weapon = NULL;	/* Which weapon he is weilding */
struct object *cur_armor = NULL;	/* the rogue's armor */
struct object *cur_ring[2];			/* Which rings are being worn */
struct stats *him;					/* pointer to hero stats */
struct trap traps[MAXTRAPS];		/* traps on this level */

int playuid;				/* uid of current player */
int playgid;				/* gid of current player */
int level = 1;				/* What level rogue is on */
int levcount = 0;			/* # of active mons this level */
int levtype = NORMLEV;		/* type of level this is, maze, etc. */
int trader = 0;				/* no. of purchases */
int curprice = -1;			/* current price of item */
int purse = 0;				/* How much gold the rogue has */
int mpos = 0;				/* Where cursor is on top line */
int ntraps;					/* # of traps on this level */
int packvol = 0;			/* volume of things in pack */
int total = 0;				/* Total dynamic memory bytes */
int demoncnt = 0;			/* number of active daemons */
int lastscore = -1;			/* Score before this turn */
int no_food = 0;			/* # of levels without food */
int seed;					/* Random number seed */
int dnum;					/* Dungeon number */
int count = 0;				/* # of times to repeat cmd */
int fung_hit = 0;			/* # of time fungi has hit */
int quiet = 0;				/* # of quiet turns */
int max_level = 1;			/* Deepest player has gone */
int food_left = HUNGERTIME;	/* Amount of food stomach */
int group = NEWGROUP;		/* Current group number */
int hungry_state = F_OKAY;	/* How hungry is he */
int foodlev = 1;			/* how fast he eats food */
int ringfood = 0;			/* rings affect on food consumption */
char take;					/* Thing the rogue is taking */
char runch;					/* Direction player is running */
char curpurch[15];			/* name of item ready to buy */

char prbuf[LINLEN];			/* Buffer for sprintfs */
char whoami[LINLEN];		/* Name of player */
char fruit[LINLEN];			/* Favorite fruit */
char huh[LINLEN];			/* The last message printed */
char file_name[LINLEN];		/* Save file name */
char scorefile[LINLEN];		/* place for scorefile */
char home[LINLEN];			/* User's home directory */
char outbuf[BUFSIZ];		/* Output buffer for stdout */

char *s_guess[MAXSCROLLS];		/* his guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* his guess at what potion is */
char *r_guess[MAXRINGS];		/* his guess at what ring is */
char *ws_guess[MAXSTICKS];		/* his guess at what wand is */

bool isfight = FALSE;		/* true if player is fighting */
bool nlmove = FALSE;		/* true when transported to new level */
bool inpool = FALSE;		/* true if hero standing in pool */
bool inwhgt = FALSE;		/* true if from wghtchk() */
bool running = FALSE;		/* True if player is running */
bool playing = TRUE;		/* True until he quits */
bool wizard = FALSE;		/* True if he is a wizard */
bool after = TRUE;			/* True if we want after daemons */
bool door_stop = FALSE;		/* Stop run when we pass a door */
bool firstmove = FALSE;		/* First move after door_stop */
bool waswizard = FALSE;		/* Was a wizard sometime */
bool amulet = FALSE;		/* He found the amulet */
bool in_shell = FALSE;		/* True if executing a shell */
bool nochange = FALSE;		/* true if last stat same as now */

bool s_know[MAXSCROLLS];		/* Does he know about a scroll */
bool p_know[MAXPOTIONS];		/* Does he know about a potion */
bool r_know[MAXRINGS];			/* Does he know about a ring */
bool ws_know[MAXSTICKS];		/* Does he know about a stick */

char spacemsg[] =	{ "-- Press space to continue --" };
char morestr[] =	{ "-- More --" };
char retstr[] =		{ "[Press return to continue]" };
char wizstr[] =		{ "Wizards Password: " };
char illegal[] =	{ "Illegal command '%s'." };
char callit[] =		{ "Call it: " };
char starlist[] =	{ " (* for a list)" };

struct coord oldpos;		/* Pos before last look() call */
struct coord delta;			/* Change indicated to get_dir() */
struct coord stairs;		/* where the stairs are put */
struct coord rndspot = { -1, -1 };	/* for random teleporting */

struct monster *mtlev[MONRANGE];

#define _r {10,10,10,10}	/* real ability (unused) */
#define _p 0,0,0,0			/* hit points, pack, carry (unused) */
#define _c 10				/* constitution (unused) */

/*
 * NAME SHOW CARRY {LEVEL} FLAGS _r {STR DEX WIS _c} EXP LVL ARM _p DMG
 */
struct monster monsters[MAXMONS + 1] = {
{"giant ant",'A',0,{3,12,1},ISMEAN,{_r,{10,16,5,_c},10,2,3,_p,"1d6"}},
{"bat",'B',0,{1,6,1},ISHUH,{_r,{10,10,10,_c},1,1,3,_p,"1d2"}},
{"centaur",'C',15,{8,17,1},0,{_r,{16,10,15,_c},15,4,4,_p,"1d6/1d6"}},
{"red dragon",'D',100,{21,500,0},ISGREED,{_r,{17,10,17,_c},9000,11,-1,_p,"1d8/1d8/3d10"}},
{"floating eye",'E',0,{2,11,0},0,{_r,{10,10,10,_c},5,1,9,_p,"0d0"}},
{"violet fungi",'F',0,{15,24,0},ISMEAN|ISSTUCK,{_r,{10,5,3,_c},85,8,2,_p,"000d0"}},
{"gnome",'G',10,{6,15,1},0,{_r,{10,10,11,_c},8,1,5,_p,"1d6"}},
{"hobgoblin",'H',0,{1,8,1},ISMEAN,{_r,{10,10,10,_c},3,1,5,_p,"1d8"}},
{"invisible stalker",'I',0,{16,25,1},ISINVIS|ISHUH,{_r,{10,15,15,_c},120,8,2,_p,"4d4"}},
{"jackal",'J',0,{1,6,1},ISMEAN,{_r,{10,10,10,_c},2,1,7,_p,"1d2"}},
{"kobold",'K',0,{1,6,1},ISMEAN,{_r,{10,10,10,_c},1,1,8,_p,"1d4"}},
{"leprechaun",'L',0,{7,16,0},0,{_r,{10,15,16,_c},10,3,8,_p,"1d1"}},
{"mimic",'M',30,{19,500,0},0,{_r,{10,10,10,_c},140,8,7,_p,"3d4"}},
{"nymph",'N',100,{11,20,0},0,{_r,{10,18,18,_c},40,3,9,_p,"0d0"}},
{"orc",'O',15,{4,13,1},0,{_r,{10,10,10,10},5,1,6,_p,"1d8"}},
{"purple worm",'P',70,{22,500,0},0,{_r,{18,5,10,_c},7000,15,6,_p,"2d12/2d4"}},
{"quasit",'Q',30,{10,19,1},ISMEAN,{_r,{10,15,16,_c},35,3,2,_p,"1d2/1d2/1d4"}},
{"rust monster",'R',0,{9,18,1},ISMEAN,{_r,{10,10,10,_c},25,5,2,_p,"0d0/0d0"}},
{"snake",'S',0,{1,7,1},ISMEAN,{_r,{10,10,10,_c},3,1,5,_p,"1d3"}},
{"troll",'T',50,{13,22,0},ISMEAN|ISREGEN,{_r,{10,10,11,_c},55,6,4,_p,"1d8/1d8/2d6"}},
{"umber hulk",'U',40,{18,500,1},ISMEAN,{_r,{17,10,10,_c},130,8,2,_p,"3d4/3d4/2d5"}},
{"vampire",'V',20,{20,500,1},ISMEAN|ISREGEN,{_r,{21,16,16,_c},380,8,1,_p,"1d10"}},
{"wraith",'W',0,{14,23,1},ISMEAN,{_r,{10,10,10,_c},55,5,4,_p,"1d6"}},
{"xorn",'X',0,{17,26,1},ISMEAN,{_r,{17,6,11,_c},120,7,-2,_p,"1d3/1d3/1d3/4d6"}},
{"yeti",'Y',30,{12,21,1},ISMEAN,{_r,{10,10,10,_c},50,4,6,_p,"1d6/1d6"}},
{"zombie",'Z',0,{5,14,1},ISMEAN,{_r,{10,10,10,_c},7,2,8,_p,"1d8"}},
{"anhkheg",'a',10,{7,16,1},ISMEAN,{_r,{10,15,3,_c},20,3,2,_p,"3d6"}},
{"giant beetle",'b',0,{9,18,1},ISMEAN,{_r,{10,15,10,_c},30,5,3,_p,"4d4"}},
{"cockatrice",'c',100,{8,17,0},0,{_r,{10,10,11,_c},200,5,6,_p,"1d3"}},
{"bone devil",'d',0,{27,500,1},ISMEAN,{_r,{18,10,16,_c},8000,12,-1,_p,"5d4"}},
{"elasmosaurus",'e',0,{28,500,1},ISMEAN,{_r,{17,5,3,_c},4500,12,7,_p,"4d6"}},
{"killer frog",'f',0,{3,8,1},ISMEAN,{_r,{10,10,10,_c},4,3,8,_p,"2d3/1d4"}},
{"green dragon",'g',50,{25,500,1},0,{_r,{18,10,18,_c},7500,10,2,_p,"1d6/1d6/2d10"}},
{"hell hound",'h',20,{10,19,1},ISMEAN,{_r,{10,15,10,_c},30,5,4,_p,"1d10"}},
{"imp",'i',20,{2,9,1},ISMEAN|ISREGEN,{_r,{10,14,11,_c},6,2,1,_p,"1d4"}},
{"jaguar",'j',0,{10,19,0},0,{_r,{10,10,11,_c},25,8,6,_p,"2d3/2d5"}},
{"koppleganger",'k',20,{8,17,1},ISMEAN,{_r,{10,10,16,_c},35,4,5,_p,"1d12"}},
{"lonchu",'l',15,{2,9,1},ISMEAN,{_r,{10,4,18,_c},5,2,1,_p,"1d4/1d4"}},
{"minotaur",'m',0,{12,21,1},ISMEAN,{_r,{10,10,11,_c},40,8,6,_p,"1d3/2d4"}},
{"neotyugh",'n',10,{14,23,1},ISMEAN,{_r,{10,6,4,_c},50,6,3,_p,"1d8/1d8/2d3"}},
{"ogre",'o',50,{7,16,1},0,{_r,{20,10,10,_c},15,4,5,_p,"2d6"}},
{"pseudo dragon",'p',50,{9,18,1},0,{_r,{10,10,16,_c},20,4,2,_p,"2d3/1d6"}},
{"quellit",'q',85,{30,500,1},0,{_r,{17,10,10,_c},12500,17,0,_p,"2d10/2d6"}},
{"rhynosphinx",'r',40,{26,500,0},0,{_r,{19,6,18,_c},5000,13,-1,_p,"2d10/2d8"}},
{"shadow",'s',15,{5,14,1},ISMEAN|ISREGEN|ISINVIS,{_r,{10,17,18,_c},6,3,5,_p,"1d6"}},
{"titanothere",'t',0,{19,500,0},0,{_r,{17,6,3,_c},750,14,6,_p,"2d8/1d6"}},
{"ulodyte",'u',10,{2,8,1},ISMEAN,{_r,{10,10,10,_c},3,2,5,_p,"1d3/1d3"}},
{"vrock",'v',0,{4,13,1},ISMEAN,{_r,{10,10,11,_c},8,3,2,_p,"1d4/1d6"}},
{"wuccubi",'w',0,{14,23,1},ISMEAN,{_r,{10,10,10,_c},90,6,0,_p,"1d4/1d10"}},
{"xonoclon",'x',0,{20,500,0},0,{_r,{19,10,4,_c},1750,14,0,_p,"3d8"}},
{"yeenoghu",'y',10,{15,24,1},ISMEAN,{_r,{17,15,10,_c},250,8,1,_p,"3d6"}},
{"zemure",'z',0,{1,6,1},ISMEAN|ISREGEN,{_r,{10,10,10,_c},4,2,7,_p,"1d4"}},
{"devil Asmodeus",'A',-1,{1,500,1},ISMEAN|ISREGEN,{_r,{24,18,18,_c},500000,40,-10,_p,"4d10/4d10"}},
};

#undef _p		/* erase these definitions */
#undef _c
#undef _r

struct h_list helpstr[] = {
	'?',	"	prints help",
	'/',	"	identify object",
	'h',	"	left",
	'j',	"	down",
	'k',	"	up",
	'l',	"	right",
	'y',	"	up & left",
	'u',	"	up & right",
	'b',	"	down & left",
	'n',	"	down & right",
	'H',	"	run left",
	'J',	"	run down",
	'K',	"	run up",
	'L',	"	run right",
	'Y',	"	run up & left",
	'U',	"	run up & right",
	'B',	"	run down & left",
	'N',	"	run down & right",
	't',	"<dir>	throw something",
	'f',	"<dir>	forward until find something",
	'p',	"<dir>	zap a wand in a direction",
	'z',	"	zap a wand or staff",
	'>',	"	go down a staircase",
	's',	"	search for trap/secret door",
	'.',	"	(dot) rest for a while",
	'i',	"	inventory pack",
	'I',	"	inventory single item",
	'q',	"	quaff potion",
	'r',	"	read a scroll",
	'e',	"	eat food",
	'w',	"	wield a weapon",
	'W',	"	wear armor",
	'T',	"	take armor off",
	'P',	"	put on ring",
	'R',	"	remove ring",
	'd',	"	drop object",
	'c',	"	call object",
	'O',	"	examine/set options",
	'a',	"	display maximum stats",
	'D',	"	dip object in pool",
	CTRL('L'),"	redraw screen",
	ESCAPE,	"	cancel command",
	'!',	"	shell escape",
	'S',	"	save game",
	'Q',	"	quit",
	0, 0
};

char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char *r_stones[MAXRINGS];		/* Stone settings of the rings */
struct rod ws_stuff[MAXSTICKS];	/* Stuff for sticks */

struct magic_item things[NUMTHINGS + 1] = {
	{ "potion",	257,	 5, },
	{ "scroll",	250,	30, },
	{ "food",	185,	 7, },
	{ "weapon",	 92,	 0, },
	{ "armor",	 92,	 0, },
	{ "ring",	 62,	 5, },
	{ "stick",	 62,	 0, },
	{ "amulet",	 0,   -250, },
	{ NULL,		 0,		 0,	},
};

struct magic_item a_magic[MAXARMORS + 1] = {
	{ "leather armor",			170,   5 },
	{ "ring mail",				130,  30 },
	{ "studded leather armor",	130,  20 },
	{ "scale mail",				120,   3 },
	{ "padded armor",			100, 250 },
	{ "chain mail",				 90,  75 },
	{ "splint mail",			 90,  80 },
	{ "banded mail",			 90,  90 },
	{ "plate mail",		 		 50, 400 },
	{ "plate armor",			 30, 650 },
	{ NULL,						  0,   0 },
};
struct init_armor armors[MAXARMORS] = {
	{ 8,	150,	500,	},
	{ 7,	250,	650,	},
	{ 7,	200,	550,	},
	{ 6,	400,	900,	},
	{ 6,	100,	450,	},
	{ 5,	300,	650,	},
	{ 4,	400,	700,	},
	{ 4,	350,	600,	},
	{ 3,	450,	950,	},
	{ 2,	350,	750,	},
};
struct magic_item w_magic[MAXWEAPONS + 1] = {
	{ "mace",				 70,  25 },
	{ "long sword",			 70,  60 },
	{ "short bow",			 60, 150 },
	{ "arrow",				 60,   2 },
	{ "dagger",				 20,   5 },
	{ "rock",				 20,   1 },
	{ "two-handed sword",	 50, 120 },
	{ "sling",				 20,   5 },
	{ "dart",				 30,   3 },
	{ "crossbow",			 60,  70 },
	{ "crossbow bolt",		 60,   3 },
	{ "spear",				 70,   8 },
	{ "trident",			 70,  90 },
	{ "spetum",				 70,  50 },
	{ "bardiche",			 70,  30 },
	{ "pike",				 70,  75 },
	{ "bastard sword",		 60, 100 },
	{ "halberd",			 70,  40 },
	{ NULL,					  0,   0 },
};

struct init_weps weaps[MAXWEAPONS] = {
	{ "2d4",  "1d3", 0, 100, 300, NONE },
	{ "1d10", "1d2", 0,  60, 180, NONE },
	{ "1d1",  "1d1", 0,  40, 190, NONE },
	{ "1d1",  "1d6", ISMANY|ISMISL, 5, 8, BOW },
	{ "1d6",  "1d4", ISMISL, 10, 30, NONE },
	{ "1d2",  "1d4", ISMANY|ISMISL, 5, 10, SLING },
	{ "3d6",  "1d2", 0, 250, 550, NONE },
	{ "0d0",  "0d0", 0,   5, 7, NONE },
	{ "1d1",  "1d3", ISMANY|ISMISL, 5, 5, NONE },
	{ "1d1",  "1d1", 0, 100, 250, NONE },
	{ "1d2", "1d10", ISMANY|ISMISL, 7, 11, CROSSBOW },
	{ "1d8",  "1d6", ISMISL, 50, 200, NONE },
	{ "3d4",  "1d4", 0,  50, 220, NONE },
	{ "2d5",  "1d3", 0,  50, 200, NONE },
	{ "3d3",  "1d2", 0, 125, 270, NONE },
	{ "1d12", "1d8", 0,  80, 260, NONE },
	{ "2d7",  "1d2", 0, 100, 400, NONE },
	{ "2d6",  "1d3", 0, 175, 370, NONE },
};

struct magic_item s_magic[MAXSCROLLS + 1] = {
	{ "monster confusion",	 50, 200 },
	{ "magic mapping",		 52, 200 },
	{ "light",				 80, 100 },
	{ "hold monster",		 25, 200 },
	{ "sleep",				 41,  50 },
	{ "enchant armor",		 75, 175 },
	{ "identify",			211, 150 },
	{ "scare monster",		 42, 300 },
	{ "gold detection",		 32, 100 },
	{ "teleportation",		 73, 200 },
	{ "enchant weapon",		 91, 175 },
	{ "create monster",		 34,  75 },
	{ "remove curse",		 82, 100 },
	{ "aggravate monsters",	 10,  50 },
	{ "blank paper",		 11,  50 },
	{ "genocide",			  5, 350 },
	{ "item knowledge",		 14, 250 },
	{ "item protection",	  9, 250 },
	{ "demons curse",		  5,  25 },
	{ "transport",			 11, 100 },
	{ "enchantment",		  3, 300 },
	{ "gods blessing",		  4, 450 },
	{ "aquirement",			  3, 450 },
	{ "banishment",			  5,  25 },
	{ "recharge wand",		 14, 250 },
	{ "locate traps",		 18, 185 },
	{ NULL,					  0,   0 },
};

struct magic_item p_magic[MAXPOTIONS + 1] = {
	{ "confusion",			 69,  50 },
	{ "paralysis",			 69,  50 },
	{ "poison",				 55,  50 },
	{ "gain strength",		130, 150 },
	{ "see invisible",		 25, 175 },
	{ "healing",			120, 130 },
	{ "monster detection",	 59, 120 },
	{ "magic detection",	 54, 105 },
	{ "raise level",		 25, 300 },
	{ "extra healing",		 52, 175 },
	{ "haste self",			 41, 200 },
	{ "restore strength",	140, 200 },
	{ "blindness",			 25,  50 },
	{ "thirst quenching",	 10,  50 },
	{ "increase dexterity",	 50, 175 },
	{ "etherealness",		 20, 150 },
	{ "increase wisdom",	 35, 175 },
	{ "regeneration",		 10, 175 },
	{ "super ability",		  3, 500 },
	{ "decrepedness",		  4,  25 },
	{ "invincibility",		  4, 500 },
	{ NULL,					  0,   0 },
};

struct magic_item r_magic[MAXRINGS + 1] = {
	{ "protection",			 71, 200 },
	{ "strength",			 70, 200 },
	{ "sustain strength",	 45, 250 },
	{ "searching",			 70, 150 },
	{ "see invisible",		 77, 175 },
	{ "constitution",		 13, 350 },
	{ "aggravate monster",	 60, 100 },
	{ "agility",			 75, 250 },
	{ "increase damage",	 61, 250 },
	{ "regeneration",		 41, 250 },
	{ "digestion",			 60, 225 },
	{ "teleportation",		 60, 100 },
	{ "stealth",			 75, 200 },
	{ "speed",				 40, 225 },
	{ "find traps",			 27, 200 },
	{ "delusion",			 18, 100 },
	{ "sustain ability",	  9, 450 },
	{ "blindness",			 10,  50 },
	{ "lethargy",			 14,  75 },
	{ "ogre strength",		  8, 350 },
	{ "enfeeblement",		  5,  25 },
	{ "burden",				 10,  50 },
	{ "illumination",		 16, 100 },
	{ "fire protection",	  5, 225 },
	{ "wisdom",				 25, 200 },
	{ "dexterity",			 35, 200 },
	{ NULL,					  0,   0 },
};

struct magic_item ws_magic[MAXSTICKS + 1] = {
	{ "light",				 95, 120 },
	{ "striking",			 75, 115 },
	{ "lightning",			 30, 200 },
	{ "fire",				 30, 200 },
	{ "cold",				 30, 200 },
	{ "polymorph",			 95, 210 },
	{ "magic missile",		 70, 170 },
	{ "haste monster",		 80,  50 },
	{ "slow monster",		 90, 220 },
	{ "drain life",			 80, 210 },
	{ "nothing",			 10,  70 },
	{ "teleport away",		 55, 140 },
	{ "teleport to",		 50,  60 },
	{ "cancellation",		 55, 130 },
	{ "sap life",			 20,  50 },
	{ "curing",				 25, 250 },
	{ "pyromania",			 15,  25 },
	{ "annihilate monster",	  5, 750 },
	{ "paralyze monster",	 10, 650 },
	{ "food absorption",	 10,  75 },
	{ "regenerate monster",	 15,  25 },
	{ "hide monster",		 10,  50 },
	{ "anti-matter",		  5,  25 },
	{ "clone monster",		 10,  10 },
	{ "confuse monster",	 15, 150 },
	{ "degenerate monster",	 15, 150 },
	{ NULL,					  0,   0 },
};

struct magic_info thnginfo[NUMTHINGS] = {
	{ MAXPOTIONS,	V_POTION,	POTION,	p_magic,	},
	{ MAXSCROLLS,	V_SCROLL,	SCROLL,	s_magic,	},
	{ MAXFOODS,		V_FOOD,		FOOD,	NULL,		},
	{ MAXWEAPONS,	V_WEAPON,	WEAPON,	w_magic,	},
	{ MAXARMORS,	V_ARMOR,	ARMOR,	a_magic,	},
	{ MAXRINGS,		V_RING,		RING,	r_magic,	},
	{ MAXSTICKS,	V_STICK,	STICK,	ws_magic,	},
	{ MAXAMULETS,	V_AMULET,	AMULET,	NULL,		},
};

long e_levels[] = {
    10L,20L,40L,80L,160L,320L,640L,1280L,2560L,5120L,10240L,20480L,
    40920L, 81920L, 163840L, 327680L, 655360L, 1310720L, 2621440L,
	3932160L, 5242880L, 7864320L, 10485760L, 15728640L, 20971520L,
	41943040L, 83886080L, 167772160L, 335544320L, 0L,
};

WINDOW *cw;		/* what the hero sees */
WINDOW *hw;		/* utility window */
WINDOW *mw;		/* monster window */
