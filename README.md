# D64BLAM

BLAM ("Basic Line Action Macro") is the macro compiler of Doom 64 maps. It compiles a macro script into the
MACROS lump of a map, and decompiles a MACROS lump back into a script. It was written by Samuel Villarreal
(Kaiser) in 2012. Doom Builder 64 and [Doom Builder 64 II](https://github.com/Styd051/Doom-Builder-64-II) run
it from their script editor ("Doom64 BLAM Script").

## History

- The first commit of this repository (`64a1966`) is the source code of BLAM as Kaiser left it.
- In 2026 it was fixed and checked against the code of the Doom 64 engine (the reverse engineering
  [DOOM64-RE](https://github.com/Erick194/DOOM64-RE) of Erick194), Doom64 EX and the 2020 remaster:
  - `c13b983`: names with digits, tabs and constants in a `#define`, checks of every limit instead of writing
    out of an array, error messages instead of crashes, exit code, CMake build;
  - `76e6de2`: the decompiler writes a text that runs as the lump does; no limit of 255 actions per macro;
  - `6d8da1b`: `loop(n)` runs its body exactly n times (it ran n + 1 times); `bin/common.txt` has the names of
    Doom Builder 64 II;
  - `06fca1d`: the line of an error in a `#define`, unknown directives, names defined twice, `#setdir`.
- A script that compiled with the version of Kaiser gives the same lump, except that its loops run once less
  (since `6d8da1b`), `Floor_RaiseToNearest` is now the special 119 (see Files), and a name defined twice
  with another text is now an error.

## Building

The code is C with no other library.

- CMake with Visual Studio: `cmake -S . -B build -A Win32`, then `cmake --build build --config Release`.
  The program is `build\Release\BLAM.exe`. `-A Win32` makes a 32-bit program, like the one of the builders.
- Visual Studio, from an x86 developer command prompt:
  `cl /O2 /W3 /D _CRT_SECURE_NO_WARNINGS /I include src\*.c /Fe:blam.exe`
- `msvc/blam.sln` is the Visual Studio 2008 project of Kaiser, kept as it was.

It is built and tested with MSVC 14.51 for x86, with no warning at `/W3`. A 64-bit build gives the same lumps,
but MSVC warns there about conversions from `size_t` to `int`. Other compilers are not tested.

## Command line

```
blam.exe <script> <lump> [-i <folder>] [-v]
blam.exe <lump> <anything> -d
```

- Without `-d`, BLAM compiles `<script>` and writes the MACROS lump into the file `<lump>`.
- `-d` decompiles the lump `<lump>` into `<lump>_decompiled.txt`. The second argument is needed but not used.
- `-i <folder>` is the folder of the files of `#include`. A separator is added at its end if it has none.
- `-v` writes a trace of the reading into `blamdebug.txt` (a large file).
- On an error, BLAM writes `blam_err.txt` in the current folder and exits with code 1. On success it exits with
  code 0 and writes no `blam_err.txt`; it does not delete an older one.
- `blam_err.txt` holds the file and the message, then the line and the position:

  ```
  s.txt : Expected a number, but found: TSEC (in Floor_MoveByValue > PushVar)
  (line = 6, pos = 32)
  ```

  For an error in the text of a `#define`, the line is the one where the name is used, and the names the
  error went through are given in brackets.
- Doom Builder 64 II copies the script, `common.txt` and `utils.txt` into a temporary folder and runs
  `blam.exe "%FI" "%FO" -i "%PT"` there.

## The language

```
#include "common.txt"
#include "utils.txt"

#define ROOM 7                      // a constant

// run by the line special 256
macro 1
{
    Door_Open(5);                   // batch 10
    wait;                           // the next batch starts when the door is open

    Floor_MoveByValue(ROOM, 64);    // batch 20: 204 : 64; 212 : 7;
    Delay(TIME_1SEC);               // 203 : 30; then a new batch

    loop(3)                         // three times
    {
        Thing_SpawnTracer(9);
        Delay(15);
    }

    52 : 0;                         // a raw action: special 52 (Exit), tag 0
}
```

**Blocks**
- `macro N { ... }` defines macro N, from 1 to 256. The line special 255 + N runs it: 256 runs macro 1,
  511 runs macro 256. A macro can be defined only once. `macro1` is read as `macro 1`.
- Comments are `// ...` and `/* ... */`.
- The words `macro`, `wait`, `loop`, `include`, `define`, `undef` and `setdir` can be written in capitals;
  the names are case sensitive.

**In a macro**
- `special : tag;` is a raw action: a line special and its tag. Both are numbers from -32768 to 32767.
- `Name;` or `Name(value, ...);` uses a `#define` (see `bin/common.txt`).
- `wait;` starts a new batch (see below).
- `loop(n) { ... }` runs its body n times, n being 1 or more. Loops cannot be nested.
- A `;` alone does nothing.
- For the actions 248, 250 and 251, the tag is the number of a macro: `Macro_Enable(3);` gives `250 : 258;`.

**Out of a macro**
- `#include "file"` reads another script: the folder of `-i`, or of `#setdir`, is put before the name.
- `#setdir "folder"` changes that folder. A separator is added at its end if it has none; `""` clears it.
- `#define Name text` and `#define Name(a, b) text` (5 values at most) define a name. The text goes to the end
  of the line; a `\` at the end of a line goes on with the next line. Every value name in the text is replaced
  by the value given. The text of a `#define` needs no `;` at its end: the one written after the name ends it.
- A name defined as a single number is a constant: it can be used wherever a number is expected.
- A name can be defined again only with the same values and the same text (a file included twice).
  `#undef Name` removes a name.
- Names have 63 characters at most, value names 15, values 63, strings 255. Strings have no escape
  characters.

A `#define` can use another one, with the values it received:

```
#define MyDoor(tag) Door_Open(tag)
#define Pair(a, b) \
    Door_Open(a); \
    Door_Close(b)
```

## The MACROS lump

Every number is a signed 16-bit little-endian number.

```
number of macros (the highest macro number of the script)
number of actions of all the macros
for each macro, from 1 to the number of macros:
    n: number of actions of the macro
    n times: batch, tag, special
    0, 0, 0
```

- The batch of the first actions of a macro is 10, and it goes up by 10 at each `wait` and around a loop.
- `loop(n) { body }` puts the body in new batches, then writes a batch of `204 : <first batch of the body>`
  and `246 : n - 1`. `loop(1)` writes nothing more than its body.
- A macro that the script does not define is written empty.

The example above gives this lump: 1 (macros), 9 (actions), then for macro 1: 9 (its actions) and

| Batch | Tag | Special | |
|---|---|---|---|
| 10 | 5 | 2 | `Door_Open(5)` |
| 20 | 64 | 204 | `Floor_MoveByValue(ROOM, 64)` |
| 20 | 7 | 212 | |
| 20 | 30 | 203 | `Delay(TIME_1SEC)` |
| 40 | 9 | 231 | `Thing_SpawnTracer(9)` |
| 40 | 15 | 203 | `Delay(15)` |
| 60 | 40 | 204 | end of `loop(3)` |
| 60 | 2 | 246 | |
| 70 | 0 | 52 | `52 : 0` |
| 0 | 0 | 0 | end of the macro |

## What the engine does with a macro

This is what the code of the Nintendo 64 engine does (DOOM64-RE: `P_LoadMacros` in `p_setup.c`, `p_macros.c`,
the actions in `P_UseSpecialLine` of `p_spec.c`). The 2020 remaster reads the lump and runs the actions 204,
246, 248, 250 and 251 in the same way. Doom64 EX differs on loops.

**Starting a macro**
- A line whose special is 256 to 511 starts macro special - 255 when it is used, crossed or shot.
- Only one macro runs at a time. A line that would start a macro while another one runs does nothing; it can
  start it later.
- A thing that dies or is picked up activates the line whose tag is its tid. While a macro runs, this waits
  in a queue of 4 and is done when the macro ends.
- An empty macro clears the special of the line that starts it.

**Batches**
- The actions of a batch run one after the other, in the same tic.
- At the end of a batch, the engine waits only if the **last** action of the batch created a thinker (a door,
  a floor, a ceiling, a platform, a delay...), until that thinker is done. Otherwise the next batch runs
  at once.
- So `Delay(30)` (`203 : 30; wait`) waits 30 tics; but in the example, batch 20 waits for its delay only,
  not for its floor.

**Actions of the macros**
- 203 creates a delay of tag tics.
- 204 sets the value that the actions with two numbers read (`Floor_MoveByValue`, `Sector_CopyLights`...),
  until the next 204.
- 246 goes back to the batch given by the value of 204:
  - on the Nintendo 64 and the remaster, the batches run tag + 1 times (a negative tag runs them for ever);
    that is why `loop(n)` writes n - 1;
  - Doom64 EX runs them tag times, and only the first loop of a macro repeats: the loops of BLAM, and those
    of the maps of the game, run once less there.
- 248 ends the macro at once.
- 250 and 251 turn a macro on and off: a macro that is off does not start.

**End of a macro**
- If the line that started it was not repeatable, its special is cleared and the macro is turned off for
  every line: it runs once in the level.

**Limits, checked by BLAM**
- 256 macros, the line specials 256 to 511.
- 32767 actions in a script: the Nintendo 64 and the remaster read their number as a signed 16-bit number
  to make room for them.
- 32766 actions in a macro: Doom64 EX keeps their number + 1 in 16 bits.
- A batch number is 16 bits too.

## Files

- `bin/common.txt` names the line actions, with the names of Doom Builder 64 II. The first names of BLAM are
  kept at the end of the file, so that older scripts still compile. One name changed: `Floor_RaiseToNearest`
  was the special 30 in the first file; it is 119 now (30 is `Floor_RaiseToTexture`).
- `PushVar` and `Delay` are used by the other names: keep them.
- `bin/utils.txt` has the `TIME_` constants and a few helpers (`Security_Camera`, `Shoot_Tracers`,
  `Shoot_Darts`, `Delayed_Exit`...).
- `bin/test.txt` is a small script.
- The decompiler writes the names of `common.txt`. It writes a raw action where a name would not run as the
  lump does: a 203 that does not end its batch, an action with two numbers with no 204 before it in its
  macro, a name without value whose tag is not 0. A decompiled text compiles back into a lump that runs as
  the first one (tested with the 39 MACROS lumps of the game).

## Credits

- BLAM was written by Samuel Villarreal (Kaiser).
- Fixed and maintained by Styd051.
- Erick194 for DOOM64-RE, the reference for the code of the Doom 64 engine.

BLAM is released under the GNU General Public License, version 2 or later (see the head of each source file).
