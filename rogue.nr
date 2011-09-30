.PH
.fp 9 CR
.nr Hu 1
.nr Pt 0
.SA 1
.S +4 C
.ad c
.B "Super-Rogue Version 9.0"
.br
.ad b
.SP 0.45
.S -4 C
.ad c
.I "Robert D. Kindelberger"
.br
.ad b
.SP 2.6
.ad c
.I "A Tutorial on the Visual Game of Rogue - Version 9.0"
.br
.ad b
.SP 2
.P
Rogue is a visual CRT based fantasy game which runs
under the UNIX\*F
.FS
UNIX is a trademark of AT&T Bell Laboratories.
.FE
timesharing system.  This paper describes
how to play rogue and gives a few hints for those who might
otherwise get lost (or killed) in the Dungeons of Doom.
.SP 5
.PH "''- % -''"
.H 1 INTRODUCTION
.P
You have just finished your years as a student
at the local fighter's guild.
After much practice and sweat you have finally completed your training and
are ready to embark upon a perilous adventure.  As a test of your skills,
the local guildmasters have sent you into the Dungeons of Doom.  Your
task is to return with the Amulet of Yendor.  
Your reward for the completion
of this task will be a full membership in the local guild. 
In addition, you are
allowed to keep all the loot you bring back from the dungeons.
.P
In preparation for your journey, you are given an enchanted weapon, taken
from a dragon's hoard in the far off Dark Mountains.  
You are also outfitted
with elf-crafted armor and given enough food to reach the dungeons.  You
say good-bye to family and friends for what may be the last time and head
up the road.
.P
You set out on your way to the dungeons and after 
several days of uneventful
travel, you see the ancient ruins that mark the entrance to the Dungeons
of Doom.  It is late at night so you make camp at the entrance and spend
the night sleeping under the open skies.  In the morning you gather
your weapon, put on your armor, eat what is almost your last food and enter
the dungeons.
.H 1 "WHAT IS GOING ON HERE?"
.P 
You have just begun a game of rogue.  
Your goal is to grab as much treasure
as you can, find the \f3Amulet of Yendor\f1, and get out of the Dungeons
of Doom alive.  
On the screen, a map of where you have been and what you have seen on
the current dungeon level is kept.  As you explore more of the level,
it appears on the screen in front of you.
.P
Rogue differs from most computer fantasy games in that it is screen
oriented.  Commands are all one or two keystrokes\*F
.FS
As opposed to pseudo English sentences.
.FE
and the results of your commands are displayed
graphically on the screen rather than being explained in words.
.P
Another major difference between rogue and other computer fantasy games
is that once you have solved all the puzzles in a standard fantasy game,
it has lost most of its excitement and it ceases to be fun.  Rogue on the
other hand generates a new dungeon every time you play it and
even the author finds it an entertaining and exciting game.
.H 1 "WHAT DO ALL THOSE THINGS ON THE SCREEN MEAN?"
.P
In order to understand what is going on in rogue you have to first get
some grasp of what rogue is doing with the screen.
The rogue screen is intended to replace the \f3You can see ...\f1 descriptions
of standard fantasy games.  Here is a sample of what a rogue screen might
look like:
.SP
.DS
\f9
                  ---------------------
                  |...................+###########+
                  |...@...........[...|
                  |........H..........|
                  |...................|
                  --------+------------
                          ####
                             +

Level: 1   Gold: 0    Hp:12(12)  Ac: 6      Exp:1/0    Vol:36%
Str:16(16) Exp:12(12) Dex:11(11) Wis:14(14) Con:18(18) Carry:50/170
\f1
.DE
.H 1 "THE BOTTOM LINES"
At the bottom line of the screen is a few pieces of cryptic information,
describing your current status.  Here is an explanation of what these
things mean:
.SP
.H 2 "LEVEL"
.SP
This number indicates how deep you have gone in the dungeon.  It starts
at one and goes up forever.\*F
.FS
Or until you get killed or decide to quit.  Level 500 is really the
maximum, but almost impossible.
.FE
.SP
.H 2 "GOLD"
.SP
The number of gold pieces you have managed to attain.
.SP
.H 2 "HP"
.SP
Your current and maximum hit points.  Hit points indicate how much
damage you can take before you die.  The more you get hit in a
fight, the lower they
get.  You can regain hit points by moving around. The number in
parentheses is the maximum number of hit points you can regain.
.SP
.H 2 "AC"
.SP
Your current armor class.  This number indicates how effective
your armor is in stopping blows from unfriendly creatures.  The lower
this number is, the more effective the armor.  Armor class can get
lower than zero.
.SP
.H 2 "EXP"
.SP
These two numbers give your current experience level and experience points.
As you kill monsters, you gain experience points.  At certain experience
point totals, you gain an experience level.  The more experienced you are,
the better you are able to fight and to withstand magical attacks. You
must gain 10 experience points to advance to the 2nd experience level.
Now you must double the previous experience point total to advance
to the next experience level. (i.e. 20 to get to level 3, 40 to 4...)
Every time you advance to a new experience level, your hit points
will increase.  This is random, so don't expect a lot every time.
.SP
.H 2 "VOL"
.SP
This is the percentage of what your pack contains.  \f3100%\f1
means that your pack is full.
.SP
.H 2 "STR"
.SP
Your current strength.  This can be any integer from 0 to 24.
The larger the number, the stronger you are.
.SP
.H 2 "DEX"
.SP
This is your dexterity.  Dexterity gives you the ability to dodge arrow
and dart traps more effectively.  It also gives you the ability to dodge
attacks from monters (maximum of 18).
.SP
.H 2 "WIS"
.SP
This is your wisdom.  If you are smarter than the monsters,
then you have a better chance of defeating them (maximum of 18).
.SP
.H 2 "CON"
.SP
This is your constitution.  Constitution makes up your ability to
regain your hit points, once you have been battered.  The higher
your constitution (maximum of 18), the faster you will gain back
the hit points you have lost.
.SP
.H 2 "CARRY"
.SP
This is similar to your volume, but tells you the maximum of what
you can carry and what you are currently carrying in weight.
.H 1 "THE TOP LINE"
.P
The top line of the screen is reserved for printing messages that describe
things that are impossible to represent visually.  If you see a
\f3-- More --\f1 on the top line, this means that rogue wants to print another
message on the screen, but it wants to make certain that you have read
the one that is there first.  To read the next message, just press a
space.
.H 1 "THE REST OF THE SCREEN"
.P
The rest of the screen is the map of the level
as you have explored it so far.
Each symbol on the screen represents something.  Here is a list of what
the various symbols mean:
.bp
.DS
.B "@   " "This symbol represents you, the adventurer."

.B "|   " "A wall running north/south."

.B "-   " "A wall running east/west."

.B "+   " "A door to/from a room."

.B "\.   " "The floor of a room."

.B "#   " "The floor of a passage between rooms."

.B "*   " "A pile or pot of gold."

.B ")   " "A weapon of some sort."

.B "]   " "A suit of armor."

.B "!   " "A flask containing a magic potion."

.B "?   " "A piece of paper, usually a magic scroll."

.B "/   " "A wand or staff of magic."

.B "=   " "A magical ring.  Can be good or bad."

.B "{   " "An arrow trap.  Loss of hit points."

.B "}   " "A bear trap.  Holds you for awhile."

.B "~   " "A teleportation trap.  Teleports you to a random room."

.B "`   " "A dart trap.  Loss of strength."

.B "$   " "A sleeping gas trap, watch out for these."

.B ">   " "A trap door leading down to the next level."

.B "^   " "A flee market to exchange gold for objects, or vice-versa."

.B "\e   " "A trap door that leads down to an invisible maze."

\f3"\f1   A magical pool that does strange things to wielded objects.

.B "%   " "The staircase leading down to the next level."

.B ":   " "A piece of food or zany fruit."

.B ",   AMULET OF YENDOR."

.B "a-Z " "There are 52 inhabitants of the Dungeons of Doom."
.DE
.bp
.H 1 COMMANDS
.P
Commands are given to rogue by pressing single letters.  Some commands can
be preceded by a count to repeat them 
(i.e. typing \f310s\f1 will do ten searches)
The list of commands is rather long, but it can be read at any time during
the game with the \f3?\f1 command.  Here it is for reference, with a short
explanation of each command:
.SP
.DS
\f3?\f1       The help command.  Asks for a character to give help on.
        If you type a \f3*\f1, it will list all the commands, otherwise
        it will explain what the character you type does.

\f3/\f1       This is the \f3What is that on the screen\f1? command.
        A \f3/\f1 followed by any character that you see on the level,
        will tell you what that character is.  For instance, typing
        \f3/@\f1 will tell you that the \f3@\f1 symbol represents you,
        the player.

\f3h, H\f1    Move left.  You move one space to the left.  If you use
        upper case, \f3H,\f1 you will continue to move left until you
        run into something.  This works for all movement commands.
        (i.e. \f3L\f1 means run in direction \f3l\f1)

.B "j, J    " "Move down."

.B "k, K    " "Move up."

.B "l, L    " "Move right."

.B "y, Y    " "Move diagonally up and left."

.B "u, U    " "Move diagonally up and right."

.B "b, B    " "Move diagonally down and left."

.B "n, N    " "Move diagonally down and right."

\f3f\f1       Find prefix.  When followed by a direction it means to
        continue moving in the specified direction until you pass
        something interesting or run into a wall.

\f3t\f1       Throw an object.  This is a prefix command.  Follow it
        with a direction and you throw an object in the specified
        direction.  (i.e. type \f3th\f1 to throw something left.)

\f3p\f1       Zap an object with a staff or wand.  This is a prefix
        command.  Follow it with a direction and you will zap in the
        specified direction.  (i.e. type \f3ph\f1 to zap something left.)
.DE
.bp
.DS
\f3z\f1       Zap an object with no direction.  This means that you
        could be zapped with it as well.

.B "D       " "Dip an object in the magic pool."

\f3>\f1       If you are standing over the passage down to the next
        level, this command means to climb down.

\f3<\f1       If you have found the \f3Amulet of Yendor\f1, then
        you have the ability to climb back up a level, hopefully on
        your way out.

\f3s\f1       Search for traps and secret doors.  Examine each space
        immediately adjacent to you for the existence of a trap or
        secret door.  There is a large chance that even if there is
        something there, you won't find it so you might have to
        search a while before you find something.

.B "\.       " "Rest.  This is the \f3do nothing\f1 command.  This is
        good for waiting and healing."

.B "i       " "Inventory.  List what you are carrying in your pack."

\f3I\f1       Selective inventory.  Tells you what a single item in
        your pack is.

.B "q       " "Quaff.  Drink one of the potions you are carrying."

.B "r       " "Read.  Read one of the scrolls in your pack."

.B "e       " "Eat food.  Take some food out of your pack and eat it."

\f3w\f1       Wield a weapon. Take a weapon out of your pack and carry
        it.  You must be wielding a weapon to use it (except to
        throw things).  To fire an arrow, you must wield the bow.
        You can only wield one weapon at a time.

\f3W\f1       Wear armor.  Take a piece of armor out of your pack and
        put it on.  You can only wear one suit of armor at a time.

.B "T       " "Take armor off.  You can't remove armor that is cursed."

\f3P\f1       Put a ring on your finger.  You can only wear two rings
        at a time.

\f3R\f1       Remove a ring from your finger.  Cursed rings are hard
        to remove.

\f3d\f1       Drop an object.  Take something out of your pack and
        leave it lying on the floor.  Only one object can occupy
        each space.
.DE
.bp
.DS
\f3O\f1       Examine and set options.  This command is further
        explained in the section on options.

\f3^l\f1      Redraws the screen.  Useful if spurious messages or
        transmission errors have messed up the display.

\f3^r\f1      Repeat last message.  Useful when a message disappears
        before you can read it.

.B "^[      " "This is the escape key.  This will cancel the last command."

\f3a\f1       Reports your encumbrance.  This is the weight factor of
        your pack.  The heavier your pack is weighted down with
        objects, the less effective you are in an attack and the
        more food you'll eat.

.B "c       " "Call an object.  You can call an object anything you like."

.B "v       " "Prints the program version number."

.B "Q       " "Quit.  Leave the game.  This is the chicken-way out."

\f3!\f1       This is the shell escape key.  Use this to get back to
        shell level.  To get back to the game, just hit \f3^d\f1.

\f3S\f1       Save the current game in a file.  Rogue won't let
        you start up a copy of a saved game, and it removes the
        save file as soon as you start up a restored game.  This
        is to prevent people from saving a game just before a
        dangerous position and then restarting it, if they die.
        Sorry no linking, copying, moving, or anything.
.DE
.bp
.H 1 "DEALING WITH OBJECTS"
.P
When you find something in the dungeon, it is common to want to pick the
object up.  This is accomplished in rogue by walking over the object.
If you are carrying too many things, you won't be able to pick the
object up.  Otherwise, the object will be added to your pack and you
will be notified of what you just picked up.
.P
Many of the commands that operate on objects must prompt you to find
out which object you want to use.  
If you change your mind and don't want to
do that command after all, just press an \f3escape\f1 key and the command
will be aborted.
.H 1 ROOMS
.P
Rooms in the dungeons are either lit or dark.  
If you walk into a lit room,
the entire room will be drawn on the screen as soon as you enter.  If you
walk into a dark room, you will only be able to see the spaces immediately
next to you.
Upon leaving a dark room, all objects inside the room are
removed from the screen.
.H 1 FIGHTING
.P
If you see a monster and you wish to fight it, just attempt to run into it.
You also may throw things at it or zap it with a wand or staff.
Many times a monster you find will mind its own business unless you attack
it.  It is often the case that discretion is the better part of valor.
.P
Smart monsters have the ability to run when they realize that their
lives are endangered.  This will be denoted that you have wounded
the monster.
.P
Monsters will avoid lit traps.  You can use this to your
advantage, if you figure out how.  If the traps are not lit, they can
fall through them as you would.  The hard part is trying to get them to
fall through a trap that you don't know is there.
.bp
.H 1 ARMOR
.P
There are various sorts of armor lying around in the dungeon.  Some of it
is enchanted, some is cursed and some is just normal.  Different armor
types have different armor classes.  The lower the armor class, the
more protection the armor affords against the blows of monsters.
If a piece of armor is enchanted or
cursed, its armor class will be higher or lower than normal.  Here is
a list of the various armor types and their normal armor class:
.DS 3
.TS
box;
c|c
l|l.
TYPE	CLASS
=
LEATHER ARMOR	8
RING MAIL	7
STUDDED LEATHER ARMOR	7
SCALE MAIL	6
PADDED ARMOR	6
CHAIN MAIL	5
SPLINT MAIL	4
BANDED MAIL	4
PLATE MAIL	3
PLATE ARMOR	2
.TE
.DE
.H 1 WEAPONS
.P
There may be many different types of weapons lying around in the dungeon,
probably left there when their previous owners ran into a monster they
couldn't handle.  In order to fire arrows and crossbow bolts you must be
wielding the bow or the crossbow.  Rocks are effective when thrown but
can be even more dangerous when hurled with a sling.  Daggers can be used
as stabbing weapons or they can be thrown.  Beware of cursed weapons,
they will not work as well as normal or enchanted weapons and once you
wield them, you will be stuck with them until you can get the curse
removed.  Staffs and wands are special weapons.  They need not be
wielded for you to use them.
.bp
.H 1 "POTIONS AND SCROLLS"
.P
Left by the wizard Yendor, the potions and scrolls found in the dungeon
constitute a mixed blessing.  By drinking or reading these magical items,
the intelligent adventurer can greatly increase his chances of survival.
A few of them can backfire on the unwary rogue, causing everything from
a weakening sickness to the creation of an angry monster.
.H 1 OPTIONS
.P
Due to variations in personal tastes and conceptions of the way rogue
should do things, there are a set of options you can set that cause
rogue to behave in various different ways.
.H 2 "SETTING THE OPTIONS"
.P
There are basically two ways to set the options.  The first is with the
\f3O\f1 command of rogue, the second is with the \f3ROGUEOPTS\f1 environment
variable.
.H 2 "USING THE O COMMAND"
.P
When you press \f3O\f1 in rogue, it clears the screen and displays the
current settings for all the options.  
It then places the cursor by the value of the
first option and waits for you to type.  
You can type a \f3RETURN\f1 which means to
go to the next option, a \f3-\f1 which means to go to the previous option,
an escape which means to return to the game, or you can give the option a
value.  For string options, type the new value followed by a return.
.H 2 "USING THE ROGUEOPTS VARIABLE"
.P
The \f3ROGUEOPTS\f1 variable is a string containing a comma
separated list of initial values for the various options.
Thus to set up an environment variable so that the name is
set to \f3Rapid Robert\f1, the fruit is \f3cherry\f1, 
and your save file is \f3fun\f1, use the command:
.SP 2
.DS
ROGUEOPTS="name=Rapid Robert,fruit=cherry,file=fun"
export ROGUEOPTS
.DE
.bp
.H 2 "OPTION LIST"
.P
Here is a list of the options and an explanation for each one.
The default value for each is enclosed in square brackets:
.BL
.LI
.B "NAME [LOGIN NAME]"
This is the name of your character.  It is used if you get on the top ten
scorer's list.  It should be less than eighty characters long.
.LI
.B "FRUIT [JUICY-FRUIT]"
This should hold the name of a 
fruit that you enjoy eating.  It is basically
a whimsy that the program uses in a couple of places.
.LI
.B "FILE [.ROGUE.SAVE]"
The default file name for saving the game.  If your phone is hung up by
accident, rogue will automatically save the game in this file.  The
file name may contain the special character \f3~\f1 which expands to be
your home directory.
.LE
.H 1 "SAVED GAMES"
.P
This is how one would restore a saved game:
.SP
.DS 1
.B "rogue .rogue.save"
.DE
.SP
This is how one would read the top ten score list of rogue:
.SP
.DS 1
.B "rogue -s"
.DE
.SP
.bp
.H 1 SYNOPSIS
.B Rogue
is a video oriented game with the object being to survive the attacks
of various monsters and gather a lot of gold.
To get started, you really only need to know two commands.  The command
.B ?
will give you a list of the available commands and the command
.B /
will identify the things you see on the screen.
.P
To win the game (as opposed to merely playing to beat other people high
scores) you must locate the
.B "Amulet of Yendor"
which is somewhere below
the \f335th\f1 level of the dungeon and get it out.
.P
When the game ends, either by your death, when you chicken-out by
quitting, or if you (by some miracle) manage to win,
.B rogue
will give you a list of what was in your pack and
will give you a list of the top ten scorers.  The scoring is based
upon how much gold you get.
If for some reason you manage to win, then the stuff in your
pack is also worth points.  The better the stuff, the more points you'll
get.  There is a \f310%\f1 penalty for getting yourself killed.
.H 1 ACKNOWLEDGEMENTS
Rogue was originally conceived of by Glenn Wichman and Michael Toy.  The
version of Super-Rogue 9.0 has been established
through the persistence of a co-worker, who would like to remain nameless.
He conceived the idea of 52 monsters, magic pools, the Flea Market,
and mazes.  Many bug fixes and joys of Super-Rogue 9.0 wouldn't
be here without the savage attack that he has put on this game.
.H 1 FILES
.BL
.LI
\f3srogue:\f1             Rogue game (object)
.LI
\f3Rogue score file search for as follows:\f1
\f3$ROGUEHOME/srogue.scr \f1
.br 
\f3/var/games/roguelike/srogue.scr \f1
.br 
\f3/var/lib/roguelike/srogue.scr \f1
.br 
\f3/var/roguelike/srogue.scr \f1
.br  .br 
\f3/usr/games/lib/srogue.scr \f1
.br 
\f3/games/roguelik/srogue.scr \f1
.br 
\f3srogue.scr:\f1 
.LI
\f3srogue.sav (user defineable):\f1    Rogue save file
.LE
.H 1 BUGS
As of the release of this memo, there are no known bugs.
Going past level 500 would be suicidal.
.bp
.H 1 "SHORT REFERENCE OF COMMANDS"
.DS
\f9
 ?       prints help               /       identify object
 h       left                      H       run left
 j       down                      J       run down
 k       up                        K       run up
 l       right                     L       run right
 y       up & left                 Y       run up & left
 u       up & right                U       run up & right
 b       down & left               B       run down & left
 n       down & right              N       run down & right
 i       inventory pack            I       one item inventory
 w       wield a weapon            W       wear armor
 a       encumbrance               O       examine/set options
 c       call object               R       remove ring
 d       drop object               P       put on ring
 e       eat food                  T       take armor off
 q       quaff potion              Q       quit game
 r       read a scroll             S       save game
 v       program version number    D       dip object in magic pool 
 z       zap a wand or staff       s       search for traps
 t <dir> throw something           f <dir> forward until find
 p <dir> directional zap           \.       rest for a while
 >       go down a staircase       <       go up a staircase
^r       repeat last message      ^l       redraw screen
^[       cancel last command      ^d       return from shell
 !       escape to shell          
\f1
.DE

