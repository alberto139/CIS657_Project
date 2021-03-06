//
//  SmartBot.cpp
//  Antz
//
//  Created by Eli Goldweber on 7/20/15.
//  Copyright (c) 2015 Zhi Xing. All rights reserved.
//

#include "SmartBot.h"
#include "Neighbor.h"
#include "Role.h"
#include "WorkerRole.h"
#include "GuiderRole.h"
#include "LineRole.h"
#include "CommTestRole.h"

using namespace Antz;

/* SmartBot -- Constructor */
SmartBot::SmartBot(uint32_t robotId):
    AntzRobot(robotId),
    curFood(NO_SIGNAL),
    curNest(NO_SIGNAL)
{
    //robotRole = new LineRole(*this); 
    //robotRole = new WorkerRole(*this); //------ CHANGE ------
  
    //robotRole = new GuiderRole(*this);
    robotRole = new CommTestRole(*this);
    seenRobots = new Dll();
}

SmartBot::~SmartBot()
{
    wipeNeighbors();
    delete robotRole;
    delete seenRobots;

    Serial.println("Exiting smarBot");
}

/* setup -- setup routine for SmartBot robot */
void SmartBot::setup()
{
    AntzRobot::setup();
    Serial.begin(9600);
}

/* loop -- loop routine for SmartBot robot */
void SmartBot::loop()
{
    if (robotRole->makeStep() == SWITCH_ROLE);
       // switchRole();
}

void SmartBot::switchRole()
{
    int roleId = robotRole->getRoleId();
    delete robotRole;
    if (roleId == ROLE_WORKER)
        robotRole = new GuiderRole(*this);
    else
        robotRole = new WorkerRole(*this);
}

void SmartBot::wipeNeighbors()
{
    for (int i = 0; i < 6; i++)
    {
        delete neighbors[i];
        neighbors[i] = NULL;
    }
    delete seenRobots;
    seenRobots = new Dll();
}

void SmartBot::registerRobotSignal(Neighbor& robot, int sensor)
{
    DllIter* iter = seenRobots->createIterator();
    Neighbor* neighbor = NULL;
    while (iter->hasNext() && neighbor == NULL)
    {
        Neighbor* candidate = iter->getNext();
        if (candidate->id == robot.id){
            neighbor = candidate;
            neighbor->count++;
        }
    }
    delete iter;
    if (neighbor == NULL)
    {
        // Page Replacement Happens Here
        // Random Page Replacement
        
        robot.receivedFrom[sensor]++; 
        
        if(seenRobots->getSize() >= 2){
          DllIter* iter = seenRobots->createIterator();
          Neighbor* current = iter->getNext();
          Neighbor* temp = current;
          current = iter->getNext();
 
          while (current != NULL)
          {
            if (temp->count > current->count){
              temp = current;
            }
            current = iter->getNext();
          }
          seenRobots->remove(*temp);
          Serial.println("************************REMOVED********************************");
        }
        
        seenRobots->pushFront(robot);
    }
    else
    {
        neighbor->receivedFrom[sensor]++;
        delete &robot;
    }
}

void SmartBot::formNeighborhood()
{
    //Serial.println(seenRobots->getSize());
    while (!seenRobots->isEmpty())
    if (true)
    {
        DllIter* iter = seenRobots->createIterator();
        Neighbor* curMax = NULL;
        int curMaxIndex = 0;
        int maxVal = 0;
        while (iter->hasNext())
        {
            Neighbor* next = iter->getNext();
            int index = next->mostlySeenFrom();

            if (next->receivedFrom[index] > maxVal)
            {
                maxVal = next->receivedFrom[index];
                curMax = next;
                curMaxIndex = index;
            }
        }

         if (curMax != NULL && maxVal > 0)
        {
            if (neighbors[curMaxIndex] == NULL)
            {
                neighbors[curMaxIndex] = curMax;
                Serial.println("Removing from the DLL 1");
                Serial.println(seenRobots->getSize());
                //Serial.println(seenRobots->remove(*curMax));
                //Serial.println(seenRobots->getSize());
            }
            else
            {
                curMax->receivedFrom[curMaxIndex] = 0;
                if (curMax->mostlySeenFrom() < 0){
                    Serial.println("Removing from the DLL 2");
                    Serial.println(seenRobots->getSize());
                    //Serial.println(seenRobots->remove(*curMax));
                    //Serial.println(seenRobots->getSize());
                }
            }
        }
        Serial.println("Almost DONE in formNeighbor");
        delete iter;
        Serial.println("DONE in formNeighbor");
    }
}

int SmartBot::countNeighbors()
{
    display.yellow(false); 
    int neighborCount = 0;
    //Printf of Neighborhood array
    Serial.println("Neighborhood array");
    for (int i = 0; i < 6; i++)
    {
        //if(neighbors[i] != NULL && neighbors[i]->id == 0)
        //    display.yellow(true); //I can see the nest!
        if (neighbors[i] == NULL)
            Serial.print("_");
        else
            Serial.print(neighbors[i]->id);
        Serial.print("  ");
        if (neighbors[i] != NULL)
            neighborCount++;
    }
    Serial.println("");
    Serial.print("# Neighbors: ");
    Serial.println(neighborCount);
    //End of Printf of Neighborhood array

//    if(neighborCount)
//        display.yellow(true);
//    else
//        display.yellow(false);
//    display.number(true, 1000);
//   delay(100);
//   display.number(true, neighborCount);
    
    return neighborCount;
    
   
}

Neighbor* SmartBot::getLowestCardNeighbor(int currentTarget, int* direction)
{
    Neighbor* neighbor = NULL;
    for (int i = 0; i < 6; i++)
        if (neighbors[i] != NULL
            && (currentTarget == TARGET_FOOD && (neighbor == NULL && neighbors[i]->curFood != NO_SIGNAL || neighbor->curFood > neighbors[i]->curFood) 
                || currentTarget == TARGET_NEST && (neighbor == NULL && neighbors[i]->curNest != NO_SIGNAL || neighbor->curNest > neighbors[i]->curNest)))
            {
                neighbor = neighbors[i];
                if(direction)
                    *direction = i;
            }
    return neighbor;
}

bool SmartBot::isNeighborValid(Neighbor& neighbor)
{
    int foodDiff = neighbor.curFood - curFood;
    int nestDiff = neighbor.curNest - curNest;
    return (neighbor.curFood == 0xFF || abs(foodDiff) <= 2 || curFood == 0xFF)
        && (neighbor.curNest == 0xFF || abs(nestDiff) <= 2 || curNest == 0xFF);
        //&& neighbor.curFood != 0 && neighbor.curNest != 0;
}
