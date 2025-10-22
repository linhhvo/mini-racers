#include "Tire.hpp"
#include <vector>
#include <set>
#include <iostream>
#include <box2d/box2d.h>

TDTire::TDTire(b2WorldId world): world_(world) {
        // Body def
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.linearDamping = 0.3f;
        bodyDef.angularDamping = 3.0f;
        body = b2CreateBody(world_, &bodyDef);

        //Box Polygon shape
        b2Polygon box = b2MakeBox(0.5f, 1.25f);

        // Tire material (can be changed for future vehicle types)
        b2SurfaceMaterial material = b2SurfaceMaterial();
        material.friction = 0.8f;
        material.restitution = 0.1f;
        material.rollingResistance = 0.1f;
        // Shape def
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1;
        shapeDef.enableContactEvents = true;
        shapeDef.enableHitEvents = true;
        shapeDef.enablePreSolveEvents = false; // true if we want to disable collision (powerup maybe?)
        shapeDef.enableSensorEvents = true;
        shapeDef.material = material;

        // Tire shape creator
        tire = b2CreatePolygonShape(body, &shapeDef, &box);

        m_currentTraction = 1;
}

void TDTire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse) {
        m_maxForwardSpeed = maxForwardSpeed;
        m_maxBackwardSpeed = maxBackwardSpeed;
        m_maxDriveForce = maxDriveForce;
        m_maxLateralImpulse = maxLateralImpulse;
    }

b2Vec2 TDTire::getLateralVelocity() const{
    b2Vec2 currentRightNormal = b2Body_GetWorldVector(body, b2Vec2{1.0f, 0.0f});
    b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
    return b2Dot( currentRightNormal, linearVel) * currentRightNormal;
}

b2Vec2 TDTire::getForwardVelocity() const{
        b2Vec2 currentForwardNormal = b2Body_GetWorldVector(body, b2Vec2{0.0f, 1.0f});
        b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
        return b2Dot( currentForwardNormal, linearVel) * currentForwardNormal;
}

void TDTire::updateFriction() {
        //lateral linear velocity
        b2Vec2 impulse = b2Body_GetMass(body) * -getLateralVelocity();
        float impulseLength = b2Length(impulse);
        if ( impulseLength > m_maxLateralImpulse )
            impulse *= m_maxLateralImpulse / impulseLength;
            impulseLength = b2Length(impulse);
        b2Body_ApplyAngularImpulse(body, m_currentTraction * impulseLength, true);

        //angular velocity
        float inertia = b2Body_GetMassData(body).rotationalInertia;
        b2Body_ApplyAngularImpulse(body, m_currentTraction * 0.1f *inertia * -b2Body_GetAngularVelocity(body), true);

        //forward linear velocity
        b2Vec2 currentForwardNormal = getForwardVelocity();
        float currentForwardSpeed = b2Length(b2Normalize(currentForwardNormal));
        float dragForceMagnitude = -2 * currentForwardSpeed;
        b2Body_ApplyForce(body, m_currentTraction * dragForceMagnitude * currentForwardNormal, b2Body_GetWorldCenterOfMass(body), true);
}

void TDTire::updateDrive(char control) {

    //find desired speed
    float desiredSpeed = 0;
    if (control == 'w') desiredSpeed = m_maxForwardSpeed;
    else if (control == 's') desiredSpeed = m_maxBackwardSpeed;
    else return; // no input

    //find current speed in forward direction
    b2Vec2 currentForwardNormal = b2Body_GetWorldVector(body, b2Vec2{1,0});
    float currentSpeed = b2Dot( getForwardVelocity(), currentForwardNormal );

    //apply necessary force
    float force = 0;
    if ( desiredSpeed > currentSpeed )
        force = m_maxDriveForce;
    else if ( desiredSpeed < currentSpeed )
        force = -m_maxDriveForce;
    else
        return;
    b2Body_ApplyForceToCenter(body, m_currentTraction * force * currentForwardNormal, true);
}

void TDTire::updateTurn(char control) {
    float desiredTorque = 0;
    if (control == 'a') {
        desiredTorque = 15;
    }
    else if (control == 'd') desiredTorque = -15;
    else return;
    b2Body_ApplyTorque(body, desiredTorque, true);
}

