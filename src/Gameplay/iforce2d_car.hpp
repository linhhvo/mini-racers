#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <box2d/box2d.h>
#include "Vehicle.hpp"


class iforce2d_TopdownCar
{
public:
    iforce2d_TopdownCar(b2WorldId world): world_(world)
    {
        b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});

        //set up ground areas
        {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            body = b2CreateBody(world_, &bodyDef);

            b2Polygon ground = b2MakeBox(9, 7);

            b2SurfaceMaterial material = b2SurfaceMaterial();
            material.friction = 0.8f;
            material.restitution = 0.1f;
            material.rollingResistance = 0.1f;
            
            // Shape def
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = 1;
            shapeDef.enableContactEvents = true;
            shapeDef.enableHitEvents = true;
            shapeDef.enableSensorEvents = true;
            shapeDef.isSensor = true;
            shapeDef.material = material;
        }

        //m_tire = new TDTire(m_world);
        //m_tire->setCharacteristics(100, -20, 150);

        car = new Vehicle(world_);

        m_controlState = 0;
    }
  
    ~iforce2d_TopdownCar()
    {
        //delete m_tire;
        delete car;
        b2DestroyBody(body);
        b2DestroyWorld(world_);
    }
    Vehicle* GetVehicle() {return car;}

    void Keyboard(unsigned char key)
    {
        switch (key) {
        case 'a' : {
            car->update('a');
            break;
        }
        case 'd' : car->update('d'); break;
        case 'w' : car->update('w'); break;
        case 's' : car->update('s'); break;
        default: iforce2d_TopdownCar::Keyboard(key);
        }
    }

    void KeyboardUp(unsigned char key)
    {
        switch (key) {
        case 'a' : car->update(0); break;
        case 'd' : car->update(0); break;
        case 'w' : car->update(0); break;
        case 's' : car->update(0); break;
        default: iforce2d_TopdownCar::Keyboard(key);
        }
    }

    static iforce2d_TopdownCar* Create(b2WorldId world)
    {
        return new iforce2d_TopdownCar(world);
    }

private:
Vehicle* car;
int m_controlState;
b2BodyId body;
b2WorldId world_;
};