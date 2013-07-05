/*
 *  Copyright 2013 Luke Puchner-Hardman
 *
 *  This file is part of Ragrelark.
 *  Ragrelark is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ragrelark is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ragrelark.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef START_H
#define START_H

#include <fstream>
#include <iomanip>
#include <stack>
#include <set>
#include <deque>
#include <cstdlib>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glx.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "fov/fov.h"

#include "Player.h"
#include "Swarmer.h"
#include "World.h"
#include "PrimeFolder.h"
#include "graphics.h"
#include "Ability.h"

#define BOOST_IOSTREAMS_NO_LIB

#define ST_CONF 20
#define ST_POIS 21
#define ST_ENCUM 22
#define ST_HUNG 23

//2000-3000 calories per day
//apple = 91 calories
//763-291-6339

#define MAX_MESSAGES 100
#define NUM_NOTELINES 26

#define INTERVAL_0_TIM 5
#define INTERVAL_1_TIM 50
#define INTERVAL_2_TIM 500
#define INTERVAL_3_TIM 5000

enum MenuAction{MA_EXAMINE, MA_GRAB, MA_DROP, MA_EQUIP, MA_EAT, MA_READ, NUM_MENU_ACTIONS};
enum{STATE_PLAY, STATE_MENU, STATE_DIR, STATE_TARGET, STATE_SPELL};
enum Panels{PANEL_EMPTY, PANEL_TOPSTART, PANEL_STATS, PANEL_SKILLS, PANEL_INVENTORY, PANEL_TOPEND, PANEL_BOTTOMSTART, PANEL_MINIMAP, PANEL_NOTES, PANEL_BOTTOMEND};
enum UnitAI{AI_STILL = 0, AI_HOSTILE = 1, AI_HOSTILESMART = 2, AI_PASSIVE = 3, AI_NEUTRAL = 4};

enum AttackType{ATT_STRIKE, ATT_SHOOT, ATT_SPELL};
enum{SA_NONE, SA_ATTACK, SA_FIRE, SA_OPENDOOR, SA_CLOSEDOOR};

typedef struct {
    int criticality;
    bool killed;
    bool hit;
    bool dodge;
} battleSummary;

class Start: FormulaUser, EnvironmentManager {
    public:
        Start();
        virtual ~Start();

        void prepare();
        void execute();

        void addMessage(std::string message, Color c);

        /* --initiator.cpp-- */
        bool init();
        /* --- */

        /* --logic.cpp-- */
        void logic();
        /* --- */

        /* --events.cpp-- */
        void events();
        void directionPress(int dir);
        void sapExp(Unit* sapper, Unit* target, SkillType skill, int multitude);
        void debankExp(Unit* debanker, SkillType skill, int amount);
        void action(SkillType skill, int exp);
        void openInventory();
        void openBag();
        void openEquipment();
        void openGround();
        void groundGrab();
        bool equipItem(Item item);
        void enterCommand();
        void backCommand();
        void closeDoors();
        void itemRemovalCheck();
        void enterTargetMode();
        void enterSpellMode();
        /* --- */

        /* --pather.cpp-- */
        void makePath(Unit* unit, Coord dest, Zone* zone, PathType special);
        void initFieldOfView();
        void playerFieldOfView(bool isNew);
        void myFovCircle(Zone* zone, void* source, Coord c, int radius);
        void myFovCirclePerm(Zone* zone, Coord c, int radius, int mod);
        void cleanFov();
        /* --- */

        /* --renderer.cpp-- */
        void render();
        void renderGround();
        void renderMenu();
        void renderSidePanels();
        void drawMenuBox(int x1, int y1, int x2, int y2);
        void renderMessages();
        void renderCircleSelect();
        void renderBars();
        void renderText(std::string text, int size, int x, int y, int z, int align, Color c);
        void startRenderer();
        void makeSplatter(Unit* unit, Zone* zone, Coord loc);
        void addStatus(std::string name, Color c, int type);
        void removeStatus(int type);
        /* --- */

        /* --particles.cpp-- */
        void createEffect(peType type, int x, int y);
        void updateEffects(int xShift, int yShift);
        void addProj(int x0, int y0, int x1, int y1, int length, int ind);
        void drawCirc(int x, int y, int z, int size, int fade, int rot, Color c);
        void drawBox(int x, int y, int z, int size, int rote, Color c);
        /* --- */

        /* --unitHandler.cpp-- */
        void ai(Unit* unit, Zone* zone);
        void followPath(Unit* unit, Zone* zone);
        void playerWalkStaminaDrain(int* movSpeed, int time, Unit* unit);
        void moveUnit(Unit* unit, Zone* zone, int dir);

        void goTheStairs(Unit* unit, Zone* zone);
        void changeLoc(Unit* unit, Zone* zone, Coord loc);
        void changeLocZ(Unit* unit, Zone* prevZone, Zone* newZone, Coord loc);

        void applyPoison(int poison, int duration, Unit* unit, Unit* poisoner);
        int actionTimePassed(int time, int speed);
        double chanceHit(double m);
        double defDam(double preDam, int defense);
        std::string defenderNoun(Unit* attacker, Unit* defender);
        Color colorHit(bool crit, bool dodge);
        void hitSapping(Unit* attacker, Unit* defender, int criticality);

        void hitCMod(Unit* defender, float& damage, float accuracy, float crit, int& hitType, battleSummary& sum);
        void strikeUnit(Unit* unit, Zone* zone, int dir, bool safe);
        void shootUnit(Unit* attacker, Unit* defender, Zone* zone);
        battleSummary attackUnit(int power, float accuracy, float crit, int weaponType, Unit* defender, Zone* zone, int dir);
        void projectItem(Item item, int power, int accuracy, Zone* zone, Coord to, Coord from);
        void reactToAttack(Unit* unit, int dir, Zone* zone);
        void killUnit(Unit* unit, Zone* zone);

        void openDoor(Unit* unit, Zone* zone, int dir, bool safe);
        void closeDoor(Unit* unit, Zone* zone, int dir, bool safe);
        void pushRock(Unit* unit, Zone* zone, int dir);
        void eatFood(Unit* unit, ItemType* food);
        void search(Unit* unit, Zone* zone);
        /* --- */

        /* --abilities.cpp-- */
        void castSpell(unsigned int spellI, Unit* unit, Zone* zone);
        /* --- */

        ItemFolder* getItemFolder(Item item);
        void createItemFolder(Item* item);
        void putItemFolder(Item* item, ItemFolder* folder);

        void addItemToPlace(Coord pos, Zone* z, Item item);
        Item removeItemFromPlace(Coord pos, Zone* z, int index);

        /* --dataLoader.cpp-- */
        void loadData(World* w, Player* p);
        void openFile(std::string fileName, World* w, Player* p);
        Zone* loadTileFile(std::string fileName, std::string zoneName);
        void finishDataSetup();
        void deleteData();
        bool errorChecker(std::string filename);
        void printFileErr(std::string said, int line);
        void printFileProb(std::string said, int line);
        /* --- */

        /* --resourceLoader.cpp-- */
        void loadImage(std::string filename);
        void buildFont();
        /* --- */

        /* --formulas.cpp-- */
        int getVarValue(VOwner target, VType type, int index, StatHolderIntef* sh);
        float getVarValueF(VOwner target, VType type, int index, StatHolderIntef* sh);
        void statChanged(int stat, StatHolderIntef* statHolder);
        void conditionChanged(int condition, StatHolderIntef* statHolder);
        int getTime();
        StatHolder* findStatHolder(int target, StatHolder* statHolder);
        void parseFormula(std::string line, bool errCheck, int lineNum);
        /* --- */

        /* --cleaner.cpp-- */
        void cleanup();
        /* --- */
    protected:
    private:
        SDL_Surface* display;
        SDL_Event event;
        Player* player;
        World* world;
        Area* activeArea;

        bool done;
        short selected;
        int state;
        int menuAction;
        std::stack<ItemFolder*> folderStack;
        PrimeFolder* primeFolder;
        std::map<unsigned short, std::vector<ItemFolder*> > folders;

        /*target & dir*/
        std::vector<Unit*> unitsInRange;
        short stIndex;
        unsigned short stateAction;
        /*endt*/
        bool circleSelect[10];
        int topPanel;
        int botPanel;
        int notesSelected;
        std::string theNotes[NUM_NOTELINES];

        bool gotsStructureTex; //TODO arrayify (with enum)
        bool gotsMenuTex;
        bool gotsFontTex;
        bool gotsSplatterTex;
        bool gotsAttackAnimsTex;
        bool gotsPlayerTex;

        RagDrawer ragd;
        RagAnim raga;

        unsigned char loadStatus;

        bool shiftIsDown;
        std::vector<std::pair<std::string, Color> > messages;

        /* temp data for loading */
        short tempValues[16];
        Coord tempCoord;
        std::string tempStr;
        std::string tempStr2;
        Zone* tempZone;
        Zone* tempZone2;
        Area* tempArea;
        DungeonStack* tempDun;

        std::map<std::string, std::vector<Zone*>*> areaZones;
        std::map<std::string, std::vector<Tile*> > tileGroups;
        std::map<std::string, int> itemTypeMap;
        std::map<std::string, int> statMap;
        std::map<std::string, int> conditionMap;
        std::map<std::string, int> spellMap;
        std::map<std::string, int> skillMap;
        std::map<std::string, Tile*> tileMap;
        std::map<std::string, MobEquipSet*> mobEquipsMap;
        std::vector<Item> itemsToEquip;
        /* end temp values for loading */

        std::vector<std::pair<StatHolder*, std::string> > spawnPrototypes;
        std::vector<int> defaultStats;
        std::vector<Unit*> unitDeleteList;

        std::map<std::string, Area*> areas;
        std::map<std::string, Zone*> zones;
        std::map<std::string, DungeonStack*> dungeons;

        std::vector<Formula*> formulas;
        int base;

        unsigned char visibilities[MAX_ZONE_SIZE]; //0 = nope, 1 = LOS, 2 = lit

        int interval0;//TODO an array
        int interval1;
        int interval2;
        int interval3;
        int foon;

        std::map<unsigned int, Tile*> tiledTiles;
        std::map<unsigned int, RandItemType*> tiledItems;
        std::map<unsigned int, int> tiledMobs;

        std::set<std::pair<Unit*, Zone*> > areaUnits; //the zone is the zone the unit is in the zone is where the unit is
        void findAreaUnits();

        long frameTime;

        MobSpawner* mobSpawner;
};

#endif // START_H
