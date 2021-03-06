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

#include <iostream>
#include "Formula.h"

int Formula::numFloats = 0;
float Formula::floats[MAX_FLOATS];

/* Create a formula with an estimated length.
 * A length too low will slow the creation of the formula while a length too high will waste memory.
 * Remember that the length of the formula is not equal to the number of commands but to the number
 * of bytes the commands use up. Put 1 if you don't know. */
Formula::Formula(int length) {
    commands.reserve(length);
}

/* There is nothing to deconstruct yet in a formula. */
Formula::~Formula() { }

/* Pushes an operator onto the stack. A list of the operator definitions is in formula.h. */
void Formula::pushOperator(FOpr op) {
    commands.push_back(1);
    commands.push_back(op);
}

/* Pushed an integer onto the stack. The int will take up...
 * ...1 byte if it is from -3 to 28. (shifted)
 * ...2 bytes if it is from -4096 to 4095. (two's comp)
 * ...5 bytes if outside that range. (aka normal int) */
void Formula::pushInt(int num) {
    if (num < 0 || num > 255) {
        commands.push_back(4);
        commands.push_back((num >> 24) & 0xFF);
        commands.push_back((num >> 16) & 0xFF);
        commands.push_back((num >> 8) & 0xFF);
        commands.push_back(num & 0xFF);
    } else {
        commands.push_back(2);
        commands.push_back(num);
    }
}

/* Pushes a float onto the stack. Can only be a max of MAX_FLOATS different floats among all formulas.
 * Takes up 1 byte in the formula. */
void Formula::pushFloat(float num) {
    commands.push_back(6);
    int i;
    for (i = 0; i < Formula::numFloats; i++) {
        if (Formula::floats[i] == num) {
            commands.push_back(i);
            return;
        }
    }
    if (Formula::numFloats == MAX_FLOATS) {
        std::cout << "maximum float values reached" << std::endl;
    } else {
        commands.push_back(Formula::numFloats);
        Formula::floats[numFloats++] = num;
    }
}

/* Pushes a variable onto the stack with the given target, type and index.
 * The types are F_STAT, F_SKILL and F_CONDITION.
 * The targets are listed in the FormulaUser.
 * Detailed info on how targets and types work is in Ragrelark.txt. */
void Formula::pushVar(int target, int type, int index) {
    //int num = 0x40 | (target << 3) | type;
    commands.push_back(8);
    commands.push_back((target << 4) | type);
    commands.push_back(index);
}

/* Runs a formula using float maths. Is slower but gets better results than run().
 * The statHolder should have the stat. The user should be the world and the prevVal should be the previous value of the stat that the formula is being run for. */
double Formula::runFloat(FormulaUser* user, StatHolderIntef* statHolder, double prevVal) {
    unsigned int went = 0; //where we are in the commands list
    double temp;
    double formulaStack[commands.size() / 2 + 1]; //the stack that values are pushed onto
    int sp = -1; //index of the top of the stack
    while(went < commands.size()) {
        unsigned char value = commands.at(went++);
        if (value == 1) {
            switch(commands.at(went++)) {
                case O_ADD : sp--;
                formulaStack[sp] = formulaStack[sp] + formulaStack[sp + 1];
                break;
                case O_SUB : sp--;
                formulaStack[sp] = formulaStack[sp] - formulaStack[sp + 1];
                break;
                case O_MUL : sp--;
                formulaStack[sp] = formulaStack[sp] * formulaStack[sp + 1];
                break;
                case O_DIV : sp--;
                formulaStack[sp] = formulaStack[sp] / formulaStack[sp + 1];
                break;
                case O_MOD : sp--;
                formulaStack[sp] = (double)((int)formulaStack[sp] % (int)formulaStack[sp + 1]);
                break;
                case O_POW : sp--;
                formulaStack[sp] = pow(formulaStack[sp], formulaStack[sp + 1]);
                break;
                case O_SWP :
                temp = formulaStack[sp];
                formulaStack[sp] = formulaStack[sp - 1];
                formulaStack[sp - 1] = temp;
                break;
                case O_SIN : formulaStack[sp] = sin(formulaStack[sp]); break;
                case O_MAX : sp--;
                if (formulaStack[sp] < formulaStack[sp + 1])
                    formulaStack[sp] = formulaStack[sp + 1];
                break;
                case O_MIN : sp--;
                if (formulaStack[sp] > formulaStack[sp + 1])
                    formulaStack[sp] = formulaStack[sp + 1];
                break;
                case O_NOT : formulaStack[sp] = !formulaStack[sp]; break;
                case O_IFE : sp--; formulaStack[sp] = (formulaStack[sp] == formulaStack[sp + 1]); break;
                case O_IFG : sp--; formulaStack[sp] = (formulaStack[sp] > formulaStack[sp + 1]); break;
                case O_TRU : formulaStack[sp] = (formulaStack[sp] == 0) ? 0 : 1; break;
                case O_SLF : formulaStack[++sp] = prevVal; break;
                case O_TIM : formulaStack[++sp] = user->getTime(); break;
                case O_EEE : formulaStack[++sp] = E; break;
                case O_PIE : formulaStack[++sp] = PI; break;
                default: std::cout << "**Error**: Not a valid operator (float)." << std::endl; break;
            }
        } else if (value == 2) {
            formulaStack[++sp] = commands.at(went++);
        } else if (value == 4) {
            formulaStack[++sp] = (double)((commands.at(went) << 24) | (commands.at(went + 1) << 16) | (commands.at(went + 2) << 8) | (commands.at(went + 3)));
            went += 4;
        } else if (value == 6) {
            formulaStack[++sp] = floats[commands.at(went++)];
        } else if (value == 8) {
            unsigned char c = commands.at(went++);
            formulaStack[++sp] = user->getVarValueF((VOwner)(c >> 4), (VType)(c & 7), commands.at(went++), statHolder);
        } else {
            std::cout << "formula error" << std::endl;
        }
    }
    return formulaStack[sp--];
}

int Formula::run(FormulaUser* user, StatHolderIntef* statHolder, int prevVal) {
    unsigned int went = 0; //step 5
    int temp;
    int formulaStack[commands.size() / 3 + 1];
    int sp = -1;
    while(went < commands.size()) {
        unsigned char value = commands.at(went++);
        if (value == 1) {
            switch(commands.at(went++)) {
                case O_ADD : sp--;
                formulaStack[sp] = formulaStack[sp] + formulaStack[sp + 1];
                break;
                case O_SUB : sp--;
                formulaStack[sp] = formulaStack[sp] - formulaStack[sp + 1];
                break;
                case O_MUL : sp--;
                formulaStack[sp] = formulaStack[sp] * formulaStack[sp + 1];
                break;
                case O_DIV : sp--;
                formulaStack[sp] = formulaStack[sp] / formulaStack[sp + 1];
                break;
                case O_MOD : sp--;
                formulaStack[sp] = formulaStack[sp] % formulaStack[sp + 1];
                break;
                case O_POW : sp--;
                formulaStack[sp] = pow(formulaStack[sp], formulaStack[sp + 1]);
                break;
                case O_SWP :
                temp = formulaStack[sp];
                formulaStack[sp] = formulaStack[sp - 1];
                formulaStack[sp - 1] = temp;
                break;
                case O_MAX : sp--;
                if (formulaStack[sp] < formulaStack[sp + 1])
                    formulaStack[sp] = formulaStack[sp + 1];
                break;
                case O_MIN : sp--;
                if (formulaStack[sp] > formulaStack[sp + 1])
                    formulaStack[sp] = formulaStack[sp + 1];
                break;
                case O_NOT : formulaStack[sp] = !formulaStack[sp]; break;
                case O_IFE : sp--; formulaStack[sp] = (formulaStack[sp] == formulaStack[sp + 1]); break;
                case O_IFG : sp--; formulaStack[sp] = (formulaStack[sp] > formulaStack[sp + 1]); break;
                case O_TRU : formulaStack[sp] = (formulaStack[sp] == 0) ? 0 : 1; break;
                case O_SLF : formulaStack[++sp] = prevVal; break;
                case O_TIM : formulaStack[++sp] = user->getTime(); break;
                default: std::cout << "**Error**: Not a valid operator (int). " << (int)value << std::endl; break;
            }
        } else if (value == 2) {
            formulaStack[++sp] = commands.at(went++);
        } else if (value == 4) {
            formulaStack[++sp] = (commands.at(went) << 24) | (commands.at(went + 1) << 16) | (commands.at(went + 2) << 8) | (commands.at(went + 3));
            went += 4;
        } else if (value == 6) {
            std::cout << "**Error**: need to use runFormulaFloat in formulas with float values " << prevVal << std::endl;
        } else if (value == 8) {
            unsigned char c = commands.at(went++);
            formulaStack[++sp] = user->getVarValue((VOwner)(c >> 4), (VType)(c & 7), commands.at(went++), statHolder);
        } else {
            std::cout << "formula error" << std::endl;
        }
    }
    return formulaStack[sp--]; //step 17
}

int Formula::getLength() {
    return commands.size();
}
