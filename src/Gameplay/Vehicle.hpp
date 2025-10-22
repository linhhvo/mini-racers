#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <box2d/box2d.h>
#include "Tire.hpp"

#ifndef DEGTORAD
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#endif

class Vehicle {
public:
    Vehicle(b2WorldId world);

    ~Vehicle() {
        for (int i = 0; i < tires.size(); i++)
            delete tires[i];
    }
    void update(char control);
    std::vector<TDTire*> GetTires() {return tires;};
    b2ShapeId GetShape() {return chassis;};
private:
    b2BodyId body;
    std::vector<TDTire*> tires;
    std::vector<b2JointId> joints;
    b2WorldId world_;
    b2ShapeId chassis;
};