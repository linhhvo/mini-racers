#pragma once
#include <vector>
#include <set>
#include <iostream>
#include <box2d/box2d.h>


enum {
    TDC_LEFT     = 0x1,
    TDC_RIGHT    = 0x2,
    TDC_UP       = 0x4,
    TDC_DOWN     = 0x8
};

class TDTire {
public:

    TDTire(b2WorldId world);

    ~TDTire() {
        b2DestroyBody(body);
    }

    b2BodyId GetBody() {return body;}

    b2ShapeId GetShape() {return tire;};

    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse);

    // TODO: Different surface handling 

    b2Vec2 getLateralVelocity() const;

    b2Vec2 getForwardVelocity() const;

    void updateFriction();

    void updateDrive(char control);

    void updateTurn(char control);

private:
    b2BodyId body;
    float m_maxForwardSpeed;
    float m_maxBackwardSpeed;
    float m_maxDriveForce;
    float m_maxLateralImpulse;
    float m_currentTraction;
    b2WorldId world_;
    b2ShapeId tire;
};