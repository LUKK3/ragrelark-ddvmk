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

#include "RagAnim.h"

RagAnim::RagAnim() {
    ragd = NULL;
    animTex = NULL;
    tileSize = 4;
}

RagAnim::RagAnim(RagDrawer* ragDrawer, Texture* animTex) {
    ragd = ragDrawer;
    this->animTex = animTex;
    tileSize = ragd->getTileSize();
}

RagAnim::~RagAnim() { }

#define TIME_MOVE_ANIM 10
#define C1 0.0
#define TIME_ATTACK_ANIM 20

int effectZInterval = 0;

double speedAnim(double timeP) {
    timeP *= 13.302;
    return (5. * (timeP + 1.) * log(timeP + 1.) - timeP * timeP / 2. - 5 * timeP) / 35.264;
}

//miss, scrape, hit, crit, megacrit
const char hTypeNumFrames[] = {4, 4, 4, 3, 1};
const char hTypeLocs[] = {0, 0, 0, 16, 28};
const char dTypeLocs[] = {32, 32, 0, 64, 32};
const char dirIndices[] = {0, 1, 2, 3, 2, 1};
const char fwubs[] = {0,  7,  0,  1,  6,  0,  2,  5,  4,  3};
void RagAnim::drawAnim(Animation* anim, int z) {
    switch(anim->type) {
        case ANIM_MOVEDIR: {
            Unit* u = anim->target;
            double x2 = anim->startX + DIRS[anim->dir].x * tileSize * anim->temp;
            double y2 = anim->ZY;
            if (u == ragd->getPlayer()->getUnit()) {
                ragd->camX = x2;
                ragd->camY = y2;
            }
            ragd->drawUnit((int)x2, (int)y2, u);
        } break;
        case ANIM_MOVELOC: {
            Unit* u = anim->target;
            double x2 = anim->startX + ((anim->value - 1) * tileSize - (anim->startX)) * anim->temp;
            double y2 = anim->ZY;
            if (u == ragd->getPlayer()->getUnit()) {
                ragd->camX = x2;
                ragd->camY = y2;
            }
            ragd->drawUnit((int)x2, (int)y2, u);
        } break;
        case ANIM_ATTACK: {
            double prorp = (double)anim->time / (double)anim->end;
            int dType = anim->value >> 4;
            int hType = anim->value & 0xF;
            int numFrames = hTypeNumFrames[hType];
            int frame = std::min((int)(prorp * 4 * numFrames), numFrames - 1);
            double florp = 1 - prorp;
            if (hType <= 1) {
                florp /= 2;
            }
            if (hType == 0) {
                glColor4f(0.5, 0.5, 0.5, florp);
            }
            glColor4f(1, 1, 1, florp);
            ragd->drawTileRot(anim->startX, anim->startY, z, animTex, dTypeLocs[dType] + hTypeLocs[hType] + frame + hTypeNumFrames[hType] * dirIndices[anim->dir % 6],
                        (anim->dir + 2) / 6, (anim->dir + 2) % 6 > 2);
            WHITE.gl();
        } break;
        case ANIM_UNIT: ragd->drawUnit(anim->startX, anim->startY, anim->target); break;
        default: break;
    }
}

void RagAnim::updateAnims() {
    /*for (set<renderable>::iterator i = renderables.begin(); i != renderables.end(); i++) {
        delete i->anim;
    }*/
    renderables.clear();
    for (unsigned int i = 0; i < anims.size(); i++) {
        Animation* anim = anims[i];
        if (anim->time >= anim->end) {
            if (anim->target) {
                if (anim->target->graphic.border == 0) std::cout << "something has gone TERRABLE wrong" << std::endl;
                anim->target->graphic.border--;
            }
            if (anim->nextAnim) {
                Animation* temp = anim;
                anim = anim->nextAnim;
                anims[i] = anim;
                delete temp;
            } else {
                delete anim;
                anims.erase(anims.begin() + i);
                i--;
                continue;
            }
        }
        anim->time++;
        int z = 0;
        switch(anim->type) {
            case ANIM_MOVEDIR: {
                float amount = speedAnim((double)anim->time / (double)anim->end);
                z = anim->startY + DIRS[anim->dir].y * tileSize * amount;
                anim->ZY = z; //the y is calculated and stored ahead of time because Z needs to be calculated early for order. X is calculated at the actual rendering
                anim->temp = amount;
                z += Z_UNIT;
            } break;
            case ANIM_MOVELOC: {
                float amount = speedAnim((double)anim->time / (double)anim->end);
                //int y = (anim->value - 1) * tileSize * amount;
                z = anim->startY + ((anim->dir - 1) * tileSize - (anim->startY)) * amount;//(anim->dir - 1) * tileSize * amount;
                anim->ZY = z;
                anim->temp = amount;
                z += Z_UNIT;
            } break;
            default: z = anim->ZY; break;
        }
        renderables.insert(Renderable(anim, z));
    }
}

void RagAnim::unitAnimTest(Unit* u, int x, int y) {
    bool has = false;
    for (unsigned int i = 0; i < anims.size(); i++) {
        int animType = anims[i]->type;
        if (animType != ANIM_ATTACK && anims[i]->target == u) {
            has = true;
            break;
        }
    }
    if (!has) {
        Animation* newAnim = new Animation;
        newAnim->type = ANIM_UNIT;
        newAnim->target = u;
        newAnim->startX = x;
        newAnim->startY = y;
        newAnim->ZY = Z_UNIT + y;
        newAnim->end = 10;
        newAnim->time = 0;
        renderables.insert(Renderable(newAnim, newAnim->ZY));
    }
}

void RagAnim::rMoveDir(Unit* unit, int dir, Coord c) {
    if (unit->graphic.border == 255) return;
    Animation* moveAnim = new Animation;
    moveAnim->type = (unsigned short)ANIM_MOVEDIR;
    moveAnim->dir = (unsigned char)dir;
    moveAnim->startX = (unsigned short)(c.x * tileSize);
    moveAnim->startY = (unsigned short)(c.y * tileSize);
    moveAnim->time = 0;
    moveAnim->end = TIME_MOVE_ANIM;
    moveAnim->target = unit;
    unit->graphic.border++;
    addAnim(moveAnim);
}

void RagAnim::rMoveLoc(Unit* unit, Coord begin, Coord end) {
    if (unit->graphic.border == 255) return;
    Animation* moveAnim = new Animation;
    moveAnim->type = (unsigned short)ANIM_MOVELOC;
    moveAnim->value = (unsigned char)(end.x + 1);
    moveAnim->dir = (unsigned char)(end.y + 1);
    moveAnim->startX = (unsigned short)(begin.x * tileSize);
    moveAnim->startY = (unsigned short)(begin.y * tileSize);
    moveAnim->time = 0;
    moveAnim->end = TIME_MOVE_ANIM;
    moveAnim->target = unit;
    unit->graphic.border++;
    addAnim(moveAnim);
}

void RagAnim::rAttack(Coord c, int dir, int dType, int hType) {
    Animation* attackAnim = new Animation;
    attackAnim->target = NULL;
    attackAnim->type = ANIM_ATTACK;
    attackAnim->dir = fwubs[dir] * 3 + rand() % 5 - 2;
    if (attackAnim->dir > 24) {
        attackAnim->dir += 24;
    }
    attackAnim->value = (dType << 4) | hType;
    int boof = 10;
    if (hType == 0) {
        boof = 30;
    }
    attackAnim->startX = c.x * tileSize + rand() % boof - (boof / 2);
    attackAnim->startY = c.y * tileSize + rand() % boof - (boof / 2);
    attackAnim->time = 0;
    attackAnim->end = TIME_ATTACK_ANIM;
    attackAnim->ZY = Z_EFFECT + effectZInterval++;
    if (effectZInterval >= 1000) {
        effectZInterval = 0;
    }
    addAnim(attackAnim);
}

void RagAnim::addAnim(Animation* anim) {
    anim->nextAnim = NULL;
    for (unsigned int i = 0; i < anims.size(); i++) {
        if (anims[i]->target && anims[i]->type == anim->type && anims[i]->target == anim->target) {
            Animation* curr = anims[i];
            while (curr->nextAnim) {
                curr = curr->nextAnim;
            }
            curr->nextAnim = anim;
            return;
        }
    }
    anims.push_back(anim);
}

void RagAnim::renderAnims() {
    for (std::set<Renderable>::iterator i = renderables.begin(); i != renderables.end(); ++i) {
        Renderable r = *i;
        drawAnim(r.anim, r.z);
        if (r.anim->type == ANIM_UNIT) {
            delete r.anim;
        }
    }
}
