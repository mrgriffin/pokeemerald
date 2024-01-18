#include "global.h"
#include "characters.h"
#include "constants/abilities.h"
#include "constants/expansion.h"
#include "constants/moves.h"
#include "constants/species.h"

// Similar to the GF ROM header, this struct allows external programs to
// detect details about Expansion.
// For this structure to be useful we have to maintain backwards binary
// compatibility. This means that we should only ever append data to the
// end. If there are any structs as members then those structs should
// not be modified after being introduced.
struct RHHRomHeader
{
    /*0x00*/ char rhh_magic[6]; // 'RHHEXP'. Useful to locate the header if it shifts.
    /*0x06*/ u8 expansionVersionMajor;
    /*0x07*/ u8 expansionVersionMinor;
    /*0x08*/ u8 expansionVersionPatch;
    /*0x09*/ u8 expansionVersionFlags;
    /*0x0A*/ u16 movesCount;
    /*0x0C*/ u16 numSpecies;
    /*0x0E*/ u16 abilitiesCount;
    /*0x10*/ const struct Ability *abilities;
    /*0x14*/ const void *metadataProgram;
    /*0x18*/ u32 metadataProgramSize;
};

#define RET 0x00
#define U8(n) 0x01, n
#define U16(n) 0x03, n
#define I32(n) 0x05, n
#define SWAP 0x07
#define FROM_POINTER 0x08
#define INDEX 0x09
#define OFFSET 0x0a
#define LOAD32 0x0b
#define LOAD8_TERMINATED 0x0c

// TODO: This should be LZ77-compressed. We need to be careful to allow
// relocations to work.
// NOTE: We would not write this struct by hand--instead we'd design a
// compiler which can produce it from a more readable representation.
static const struct __attribute__((packed)) {
    u8 cmd0; const void *op0;
    u8 cmd1;
    u8 cmd2;
    u8 cmd3; u8 op1;
    u8 cmd4;
    u8 cmd5; u8 op2;
    u8 cmd6;
    u8 cmd7;
    u8 cmd8;
    u8 cmd9; u8 op3;
    u8 cmd10;
    u8 cmd11;
} sMetadataProgram =
{
    // input: ability number
    I32(gAbilities),
    FROM_POINTER,
    SWAP,
    U8(sizeof(*gAbilities)),
    INDEX,
    U8(offsetof(struct Ability, description)),
    OFFSET,
    LOAD32,
    FROM_POINTER,
    U8(EOS),
    LOAD8_TERMINATED,
    RET,
};

__attribute__((section(".text.consts")))
static const struct RHHRomHeader sRHHRomHeader =
{
    .rhh_magic = { 'R', 'H', 'H', 'E', 'X', 'P' },
    .expansionVersionMajor = EXPANSION_VERSION_MAJOR,
    .expansionVersionMinor = EXPANSION_VERSION_MINOR,
    .expansionVersionPatch = EXPANSION_VERSION_PATCH,
    .expansionVersionFlags = (EXPANSION_TAGGED_RELEASE << 0),
    .movesCount = MOVES_COUNT,
    .numSpecies = NUM_SPECIES,
    .abilitiesCount = ABILITIES_COUNT,
    .abilities = gAbilities,
    .metadataProgram = &sMetadataProgram,
    .metadataProgramSize = sizeof(sMetadataProgram),
};
