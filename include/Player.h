#ifndef PLAYER_H
#define PLAYER_H

#include "Area.h"
#include "PrimeFolder.h"
#include "Ability.h"

#define NUM_SKILLS 56

const string skillNames[NUM_SKILLS] =
    {"athleticism", "melee combat", "unarmed combat", "lifting", "fortitude", "resist poison", "resist burn", "resist cold", "channeling", "quick casting", "concentration", "aff4",
    "learning", "knowledge", "int3", "int4", "searching", "sight", "hearing", "far shot", "dodge", "ranged combat", "throwing", "critical",
    "bartering", "intimidation", "charm", "cha4", "armor", "shields", "def3", "def4",
    "bashing", "piercing", "slashing", "special combat", "water", "fire", "air", "earth", "necromancy", "illusion", "summoning", "enchantment",
    "axe", "sword", "scythe", "hammer/maces", "staff", "flailing", "bow", "crossbow", "firearms", "spear", "dagger", "trident"};

typedef struct {
    unsigned char* bottomTex;
    unsigned char* bottomLoc;
    unsigned char* topTex;
    unsigned char* topLoc;
} zoneMemory;

typedef struct {
    int exp;
    unsigned short level;
} playerSpell;

class Player {
    public:
        Player(PrimeFolder* primeFolder);
        virtual ~Player();

        Zone* getZone();
        Area* getArea();
        Unit* getUnit();
        PrimeFolder* getPrimeFolder();
        void setZone(Zone* z);
        void setArea(Area* a);
        void setUnitProto(StatHolder* p);
        void setName(string name);

        pair<int, int> getMemoryBottom(int x, int y);
        pair<int, int> getMemoryTop(int x, int y);
        void setMemory(int x, int y, unsigned char bt, unsigned char bl, unsigned char tt, unsigned char tl);

        int gainSkillExp(SkillType skill, int xpGained); //returns the change in the skills level after the xp gain
        unsigned short getSkillLevel(SkillType skill);
        int getSkillExpPercent(SkillType skill);

        int getSpellLevel(int spellIndex);
        void trainSpell(int spellIndex, int xpGained);
        const map<int, playerSpell>::iterator getSpellsBegin();
        const map<int, playerSpell>::iterator getSpellsEnd();

        int takeFromXpBank(int amount);
        void bankXp(int amount);
        int getXpBank();
    protected:
    private:
        string name;
        Unit* unit;
        Zone* zone;
        Area* area;
        PrimeFolder* primeFolder;
        map<Zone*, zoneMemory*> memoryBank;
        zoneMemory* currentZoneMemory;
        int skillExps[NUM_SKILLS];
        unsigned short skillLevels[NUM_SKILLS];
        map<int, playerSpell> playerSpells;

        int xpBank;
};

#endif // PLAYER_H
