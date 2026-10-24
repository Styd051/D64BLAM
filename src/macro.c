// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#include "types.h"
#include "common.h"
#include "macro.h"
#include "parse.h"
#include "script.h"

macrodef_t macrodefs[MAX_MACRO_DEFS];
macrodef_t *activemacro;
macrodata_t *macroaction;

int m_activemacros;
int m_totalactions;
int m_currentline;

static int m_currentmacro;
static dboolean m_defined[MAX_MACRO_DEFS];

//
// M_NewMacro
//

void M_NewMacro(int id)
{
    if(id < 1 || id > MAX_MACRO_DEFS)
    {
        SC_Error("Macro %i is out of range (1 to %i)", id, MAX_MACRO_DEFS);
    }

    if(m_defined[id-1])
    {
        SC_Error("Macro %i is defined twice", id);
    }

    m_defined[id-1] = true;
    m_currentmacro = id;

    activemacro = &macrodefs[id-1];
    macroaction = macrodefs[id-1].data;
    activemacro->actions = 0;
    m_currentline = 10;

    if(id > m_activemacros)
        m_activemacros = id;
}

//
// M_AppendAction
//

void M_AppendAction(int type, int tag)
{
    macrodata_t* macro;

    if(type == 248 || type == 250 || type == 251)
    {
        tag = 256 + (tag-1);

        if(tag > 32767)
        {
            SC_Error("Macro %i of action %i is out of range", tag - 255, type);
        }
    }

    if(activemacro->actions >= MAX_MACRO_DATA)
    {
        SC_Error("Macro %i has more than %i actions", m_currentmacro, MAX_MACRO_DATA);
    }

    if(m_totalactions >= 32767)
    {
        SC_Error("Too many actions in the script (32767 at most)");
    }

    // the number of a wait is 16 bits too
    if(m_currentline > 32767)
    {
        SC_Error("Too many waits in macro %i", m_currentmacro);
    }

    macro = &macroaction[activemacro->actions++];
    macro->special = type;
    macro->id = m_currentline;
    macro->tag = tag;

    m_totalactions++;
}

//
// M_NewLine
//

void M_NewLine(void)
{
    m_currentline += 10;
}

//
// M_GetLine
//

int M_GetLine(void)
{
    return m_currentline;
}

//
// M_FinishMacro
//

void M_FinishMacro(void)
{
    m_currentline = 0;
}

//
// M_WriteMacroLump
//

void M_WriteMacroLump(void)
{
    int i;
    int j;

    Com_SetWriteBinaryFile("%s", myargv[2]);

    Com_Write16(m_activemacros);
    Com_Write16(m_totalactions);

    for(i = 0; i < m_activemacros; i++)
    {
        Com_Write16(macrodefs[i].actions);

        for(j = 0; j < macrodefs[i].actions; j++)
        {
            Com_Write16(macrodefs[i].data[j].id);
            Com_Write16(macrodefs[i].data[j].tag);
            Com_Write16(macrodefs[i].data[j].special);
        }

        Com_Write16(0);
        Com_Write16(0);
        Com_Write16(0);
    }

    Com_CloseFile();

    Com_Printf("Wrote %s\n", myargv[2]);
    Com_Printf("Total Macros: %i\n", m_activemacros);
    Com_Printf("Total Actions: %i\n", m_totalactions);
}

void M_Init(void)
{
    activemacro = NULL;
    macroaction = NULL;
    m_activemacros = 0;
    m_totalactions = 0;
    m_currentline = 0;
    m_currentmacro = 0;
    memset(macrodefs, 0, sizeof(macrodefs));
    memset(m_defined, 0, sizeof(m_defined));
}

typedef struct
{
    char *name;
    int args;
} m_decmp_table_t;

#define MAX_LINE_SPECIALS   255

static m_decmp_table_t decmp_table[MAX_LINE_SPECIALS] =
{
    { "No_Op", 0 },                     // 0
    { NULL, 0 },                        // 1
    { "Door_Open", 1 },                 // 2
    { "Door_Close",1 },                 // 3
    { "Door_Raise",1 },                 // 4
    { "Floor_Raise", 1 },               // 5
    { "Ceiling_RaiseCrush", 1 },        // 6
    { NULL, 0 },                        // 7
    { "Stairs_Build", 1 },              // 8
    { NULL, 0 },                        // 9
    { "Plat_DownWaitUp", 1 },           // 10
    { NULL, 0 },                        // 11
    { NULL, 0 },                        // 12
    { NULL, 0 },                        // 13
    { NULL, 0 },                        // 14
    { NULL, 0 },                        // 15
    { "Door_CloseWait30Open", 1 },      // 16
    { NULL, 0 },                        // 17
    { NULL, 0 },                        // 18
    { "Floor_Lower", 1 },               // 19
    { NULL, 0 },                        // 20
    { NULL, 0 },                        // 21
    { "Plat_RaiseChange", 1 },          // 22
    { NULL, 0 },                        // 23
    { NULL, 0 },                        // 24
    { "Ceiling_RaiseCrushOnce", 1 },    // 25
    { NULL, 0 },                        // 26
    { NULL, 0 },                        // 27
    { NULL, 0 },                        // 28
    { NULL, 0 },                        // 29
    { "Floor_RaiseToNearest", 1 },      // 30
    { NULL, 0 },                        // 31
    { NULL, 0 },                        // 32
    { NULL, 0 },                        // 33
    { NULL, 0 },                        // 34
    { NULL, 0 },                        // 35
    { "Floor_LowerFast", 1 },           // 36
    { "Floor_LowerChange", 1 },         // 37
    { "Floor_LowerToLowest", 1 },       // 38
    { "Teleport_ToDest", 1 },           // 39
    { NULL, 0 },                        // 40
    { NULL, 0 },                        // 41
    { NULL, 0 },                        // 42
    { "Ceiling_LowerToFloor", 1 },      // 43
    { "Ceiling_RaiseCrush2", 1 },       // 44
    { NULL, 0 },                        // 45
    { NULL, 0 },                        // 46
    { NULL, 0 },                        // 47
    { NULL, 0 },                        // 48
    { NULL, 0 },                        // 49
    { NULL, 0 },                        // 50
    { NULL, 0 },                        // 51
    { "Exit", 0 },                      // 52
    { "Plat_PerpetualRaise", 1 },       // 53
    { "Plat_Stop", 1 },                 // 54
    { NULL, 0 },                        // 55
    { "Floor_RaiseCrush", 1 },          // 56
    { "Ceiling_StopCrusher", 1 },       // 57 
    { "Floor_Raise24", 1 },             // 58
    { "Floor_Raise24Change", 1 },       // 59
    { NULL, 0 },                        // 60
    { NULL, 0 },                        // 61
    { NULL, 0 },                        // 62
    { NULL, 0 },                        // 63
    { NULL, 0 },                        // 64
    { NULL, 0 },                        // 65
    { "Plat_DownWaitUp24", 1 },         // 66
    { "Plat_DownWaitUp32", 1 },         // 67
    { NULL, 0 },                        // 68
    { NULL, 0 },                        // 69
    { NULL, 0 },                        // 70
    { NULL, 0 },                        // 71
    { NULL, 0 },                        // 72
    { NULL, 0 },                        // 73
    { NULL, 0 },                        // 74
    { NULL, 0 },                        // 75
    { NULL, 0 },                        // 76
    { NULL, 0 },                        // 77
    { NULL, 0 },                        // 78
    { NULL, 0 },                        // 79
    { NULL, 0 },                        // 80
    { NULL, 0 },                        // 81
    { NULL, 0 },                        // 82
    { NULL, 0 },                        // 83
    { NULL, 0 },                        // 84
    { NULL, 0 },                        // 85
    { NULL, 0 },                        // 86
    { NULL, 0 },                        // 87
    { NULL, 0 },                        // 88
    { NULL, 0 },                        // 89
    { NULL, 0 },                        // 90
    { NULL, 0 },                        // 91
    { NULL, 0 },                        // 92
    { "Thing_ModifyFlags", 1 },         // 93
    { "Thing_Alert", 1 },               // 94
    { NULL, 0 },                        // 95
    { NULL, 0 },                        // 96
    { NULL, 0 },                        // 97
    { NULL, 0 },                        // 98
    { NULL, 0 },                        // 99
    { "Stairs_Build16Fast", 1 },        // 100
    { NULL, 0 },                        // 101
    { NULL, 0 },                        // 102
    { NULL, 0 },                        // 103
    { NULL, 0 },                        // 104
    { NULL, 0 },                        // 105
    { NULL, 0 },                        // 106
    { NULL, 0 },                        // 107
    { "Door_RaiseFast", 1 },            // 108
    { "Door_OpenFast", 1 },             // 109
    { "Door_CloseFast", 1 },            // 110
    { NULL, 0 },                        // 111
    { NULL, 0 },                        // 112
    { NULL, 0 },                        // 113
    { NULL, 0 },                        // 114
    { NULL, 0 },                        // 115
    { NULL, 0 },                        // 116
    { NULL, 0 },                        // 117
    { NULL, 0 },                        // 118
    { "Floor_RaiseToNearest2", 1 },     // 119
    { NULL, 0 },                        // 120
    { "Plat_DownWaitUpFast", 1 },       // 121
    { "Plat_UpWaitDown", 1 },           // 122
    { "Plat_UpWaitDownFast", 1 },       // 123
    { "ExitToLevel", 1 },               // 124
    { NULL, 0 },                        // 125
    { NULL, 0 },                        // 126
    { NULL, 0 },                        // 127
    { NULL, 0 },                        // 128
    { NULL, 0 },                        // 129
    { NULL, 0 },                        // 130
    { NULL, 0 },                        // 131
    { NULL, 0 },                        // 132
    { NULL, 0 },                        // 133
    { NULL, 0 },                        // 134
    { NULL, 0 },                        // 135
    { NULL, 0 },                        // 136
    { NULL, 0 },                        // 137
    { NULL, 0 },                        // 138
    { NULL, 0 },                        // 139
    { NULL, 0 },                        // 140
    { "Ceiling_SilentCrusher", 1 },     // 141
    { NULL, 0 },                        // 142
    { NULL, 0 },                        // 143
    { NULL, 0 },                        // 144
    { NULL, 0 },                        // 145
    { NULL, 0 },                        // 146
    { NULL, 0 },                        // 147
    { NULL, 0 },                        // 148
    { NULL, 0 },                        // 149
    { NULL, 0 },                        // 150
    { NULL, 0 },                        // 151
    { NULL, 0 },                        // 152
    { NULL, 0 },                        // 153
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { "Camera_Clear", 1 },              // 200
    { "Camera_Set", 1 },                // 201
    { "Thing_SpawnDart", 1 },           // 202
    { "Delay", 1 },                     // 203
    { NULL, 0 },                        // 204
    { "Sector_SetFloorColorID", 2 },    // 205
    { "Sector_SetCeilingColorID", 2 },  // 206
    { "Sector_SetThingColorID", 2 },    // 207
    { "Sector_SetUpperWallColorID", 2 },// 208
    { "Sector_SetLowerWallColorID", 2 },// 209
    { "Ceiling_MoveByValue", 2 },       // 210
    { NULL, 0 },
    { "Floor_MoveByValue", 2 },         // 212
    { NULL, 0 },
    { "Elevator_MoveByValue", 2 },      // 214
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { "Line_CopyFlags", 2 },            // 218
    { "Line_CopyTextures", 2 },         // 219
    { "Sector_CopyFlags", 2 },          // 220
    { "Sector_CopySpecials", 2 },       // 221
    { "Sector_CopyLights", 2 },         // 222
    { "Sector_CopyTextures", 2 },       // 223
    { "Thing_Spawn", 1 },               // 224
    { "Quake", 1 },                     // 225
    { "Ceiling_MoveByValueFast", 2 },   // 226
    { "Ceiling_MoveByValueInstant", 2 },// 227
    { "Floor_MoveByValueFast", 2 },     // 228
    { "Floor_MoveByValueInstant", 2 },  // 229
    { "Line_CopySpecials", 2 },         // 230
    { "Thing_SpawnTracer", 1 },         // 231
    { "Ceiling_RaiseCrushFast", 1 },    // 232
    { "Player_Freeze", 1 },             // 233
    { "SetLightID", 2 },                // 234
    { "Sector_CopyLightsAndInterpolate", 2 },// 235
    { "Plat_DownUpByValue", 2 },        // 236
    { "Plat_DownUpFastByValue", 2 },    // 237
    { "Plat_UpDownByValue", 2 },        // 238
    { "Plat_UpDownFastByValue", 2 },    // 239
    { "Line_TriggerRandomLinesByTag", 1 },  // 240
    { "Pillar_OpenByValue", 2 },        // 241
    { "Thing_Dissolve", 1 },            // 242
    { "Camera_MoveAndAim", 2 },         // 243
    { "Floor_SetHeight", 2 },           // 244
    { "Ceiling_SetHeight", 2 },         // 245
    { NULL, 0 },
    { "Floor_MoveByHeight", 2 },        // 247
    { "Macro_Suspend", 1 },             // 248                        
    { "Teleport_Stomp", 1 },            // 249
    { "Macro_Enable", 1 },              // 250                        
    { "Macro_Disable", 1 },             // 251                        
    { "Ceiling_MoveByHeight", 2 },      // 252
    { "UnlockCheatMenu", 0 },           // 253
    { "DisplaySkyLogo", 0 },            // 254
};

//
// M_CheckMacroLump
// The lump must hold all that its counts say
//

static dboolean M_CheckMacroLump(byte *buffer, int len)
{
    int nummacros;
    int count;
    int offset;
    int i;

    if(len < 4)
    {
        return false;
    }

    nummacros = (short)(buffer[0] | (buffer[1] << 8));

    if(nummacros < 0)
    {
        return false;
    }

    offset = 4;

    for(i = 0; i < nummacros; i++)
    {
        if(offset + 2 > len)
        {
            return false;
        }

        count = (short)(buffer[offset] | (buffer[offset+1] << 8));

        if(count < 0)
        {
            return false;
        }

        offset += 2 + (count + 1) * (int)sizeof(macrodata_t);

        if(offset > len)
        {
            return false;
        }
    }

    return true;
}

//
// M_DecompileMacro
//

void M_DecompileMacro(const char *file)
{
    byte *buffer;
    int nummacros;
    int numactions;
    int len;
    int i;
    int j;
    int scriptline;
    int scriptvar = 0;

    len = Com_ReadBinaryFile(file, &buffer);

    if(!M_CheckMacroLump(buffer, len))
    {
        Com_Error("%s is not a valid MACROS lump", file);
    }

    Com_Printf("--------Decompiling %s--------\n", file);

    nummacros = Com_Read16(buffer);
    Com_Read16(buffer);
    Com_Printf("%i macros...\n", nummacros);

    Com_SetWriteFile("%s_decompiled.txt", file);
    Com_FPrintf("#include \"common.txt\"\n\n");

    for(i = 0; i < nummacros; i++)
    {
        int line = 0;
        int spec = 0;
        int tag = 0;
        int exitloopline;
        dboolean enterloop;
        macrodata_t *data;

        Com_FPrintf("//====================  macro %02d  ====================\n\n", i+1);
        Com_FPrintf("macro %i\n{\n", i+1);

        scriptline = 0;
        numactions = Com_Read16(buffer);
        data = (macrodata_t*)Com_GetFileBuffer(buffer);
        enterloop = false;
        exitloopline = 0;

        for(j = 0; j < numactions; j++)
        {
            int x;
            m_decmp_table_t *table;

            line = Com_Read16(buffer);

            if(j > 0 && line != scriptline)
            {
                if(spec != 203 && spec != 246)
                {
                    if(enterloop)
                    {
                        Com_FPrintf("    ");
                    }

                    Com_FPrintf("    wait;\n");
                }

                if(enterloop && exitloopline == line)
                {
                    Com_FPrintf("    }");
                    enterloop = false;
                }

                Com_FPrintf("\n");
                scriptline = line;
            }
            else if(j == 0)
            {
                scriptline = line;
            }

            if(!enterloop)
            {
                for(x = j; x < numactions; x++)
                {
                    if(data[x].special == 246)
                    {
                        if(x > 0 && data[x-1].tag == line)
                        {
                            enterloop = true;
                            exitloopline = data[x].id;
                            Com_FPrintf("    loop(%i)\n    {\n", data[x].tag);
                            break;
                        }
                    }
                }
            }

            tag = Com_Read16(buffer);
            spec = Com_Read16(buffer);

            if(spec == 204)
            {
                scriptvar = tag;
                continue;
            }
            else if(spec == 246)
            {
                continue;
            }
            else if(spec == 248 || spec == 250 || spec == 251)
            {
                tag -= 255;
            }

            table = (spec >= 0 && spec < MAX_LINE_SPECIALS) ? &decmp_table[spec] : NULL;

            if(table == NULL || table->name == NULL)
            {
                Com_FPrintf("    %i : %i;\n", spec, tag);
            }
            else
            {
                if(enterloop)
                {
                    Com_FPrintf("    ");
                }

                if(table->args == 1)
                {
                    Com_FPrintf("    %s(%i);\n",
                        table->name, tag);
                }
                else if(table->args == 2)
                {
                    Com_FPrintf("    %s(%i, %i);\n",
                        table->name, tag, scriptvar);
                }
                else
                {
                    Com_FPrintf("    %s;\n", table->name);
                }
            }
        }

        Com_FPrintf("}\n\n");

        Com_Read16(buffer);
        Com_Read16(buffer);
        Com_Read16(buffer);
    }

    Com_CloseFile();
    Com_Printf("Written %s_decompiled.txt...\n", file);
}
