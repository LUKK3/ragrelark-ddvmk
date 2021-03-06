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

#include "BagFolder.h"

BagFolder::BagFolder(int s) {
    size = s;
    items.reserve(size);
}
BagFolder::~BagFolder() {}

int BagFolder::getNumItems() {
    return items.size();
}

Item* BagFolder::getItem(int index) {
    return &items[index];
}

Item* BagFolder::getItems() {
    return &items[0];
}

Item BagFolder::removeItem(int index) {
    Item item = items[index];
    items.erase(items.begin() + index);
    return item;
}

bool BagFolder::addItem(Item* item) {
    int stackAmount = item->getType()->getStack();
    if (stackAmount > 1) {
        for (unsigned int i = 0; i < items.size(); i++) {
            Item* thisItem = &items[i];
            if (thisItem->itemType == item->itemType && thisItem->form == item->form && thisItem->quantityCharge < stackAmount) {
                if (thisItem->quantityCharge == 0) thisItem->quantityCharge++;
                if (item->quantityCharge == 0) item->quantityCharge++;
                thisItem->quantityCharge += item->quantityCharge;
                if (thisItem->quantityCharge > stackAmount) {
                    item->quantityCharge = thisItem->quantityCharge - stackAmount;
                    thisItem->quantityCharge = stackAmount;
                    //items.push_back(*item);
                } else {
                    return true;
                }
            }
        }
    }
    if (items.size() < size) {
        items.push_back(*item);
        return true;
    }
    return false;
}

int BagFolder::getCapacity() {
    return size;
}

void BagFolder::setCapacity(int c) {
    size = c;
}

void BagFolder::save(std::ostream& saveData) {
    outSht(size, saveData);
    outSht(items.size(), saveData);
    for (unsigned int i = 0; i < items.size(); i++) {
        items[i].save(saveData);
    }
}

void BagFolder::load(std::istream& saveData) {
    size = inSht(saveData);
    unsigned short numItems = inSht(saveData);
    items.resize(numItems);
    for (unsigned int i = 0; i < numItems; i++) {
        items[i] = Item(saveData);
    }
}
