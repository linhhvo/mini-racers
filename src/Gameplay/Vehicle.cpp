#include "Vehicle.hpp"
#include "Tire.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <box2d/box2d.h>

const b2RevoluteJointDef createCustomJoint(b2BodyId chassis, b2BodyId Tire, b2Vec2 Tirepos, bool isFrontTire) {
    b2RevoluteJointDef JointDef = b2DefaultRevoluteJointDef();
    JointDef.base.bodyIdA = chassis;
    JointDef.base.bodyIdB = Tire;
    JointDef.base.localFrameA.p = Tirepos;
    JointDef.base.localFrameA.q = b2MakeRot(0);
    JointDef.base.localFrameB.p = b2Vec2 {0,0};
    JointDef.base.localFrameB.q = b2MakeRot(0);
        
    JointDef.enableSpring = true;
    JointDef.hertz = 6.0;
    JointDef.enableLimit = true;
    JointDef.maxMotorTorque = 25.0f;
    JointDef.enableMotor = isFrontTire;
    return JointDef;
    }

static inline float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

Vehicle::Vehicle(b2WorldId world): world_(world) {

        //create chassis body
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.angularDamping = 3.0f;
        bodyDef.linearDamping = 0.3f;
        bodyDef.position = b2Vec2{5.0f,5.0f};
        body = b2CreateBody(world, &bodyDef);

         //Box Polygon shape
        b2Polygon box = b2MakeBox(1.0f, 2.0f);

        // Tire material (can be changed for future vehicle types)
        b2SurfaceMaterial material = b2SurfaceMaterial();
        material.friction = 0.8f;
        material.restitution = 0.0f;
        material.rollingResistance = 0.0f;
        // Shape def
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1;
        shapeDef.enableContactEvents = true;
        shapeDef.enableHitEvents = true;
        shapeDef.enablePreSolveEvents = false; // true if we want to disable collision (powerup maybe?)
        shapeDef.enableSensorEvents = true;
        shapeDef.material = material;

        // Tire shape creator
        chassis = b2CreatePolygonShape(body, &shapeDef, &box);

        //prepare common joint parameters

        float maxForwardSpeed = 250;
        float maxBackwardSpeed = -40;
        float backTireMaxDriveForce = 1300;
        float frontTireMaxDriveForce = 1500;
        float backTireMaxLateralImpulse = 8.5;
        float frontTireMaxLateralImpulse = 7.5;

        std::vector<b2Vec2> wheelOffsets = {
            {-0.5f,  0.75f},  // front-left
            { 0.5f,  0.75f},  // front-right
            {-0.5f, -0.75f},  // rear-left
            { 0.5f, -0.75f}   // rear-right
        };
        //front left tire
        TDTire* fltire = new TDTire(world);
        fltire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        const b2RevoluteJointDef flWheelDef = createCustomJoint(body, fltire->GetBody(), b2Vec2{-0.5f, 0.75f}, true);
        auto flWheel = b2CreateRevoluteJoint(world_, &flWheelDef);
        tires.push_back(fltire);
        joints.push_back(flWheel);

        //front right tire
        TDTire* frtire = new TDTire(world);
        frtire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        const b2RevoluteJointDef frWheelDef = createCustomJoint(body, frtire->GetBody(), b2Vec2{0.5f, 0.75f}, true);
        auto frWheel = b2CreateRevoluteJoint(world_, &frWheelDef);
        tires.push_back(frtire);
        joints.push_back(frWheel);


        //back left tire
        TDTire* bltire = new TDTire(world);
        bltire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        const b2RevoluteJointDef blWheelDef = createCustomJoint(body, bltire->GetBody(), b2Vec2{-0.5f, -0.75f}, true);
        auto blWheel = b2CreateRevoluteJoint(world_, &blWheelDef);
        tires.push_back(bltire);
        joints.push_back(blWheel);

        //back right tire
        TDTire* brtire = new TDTire(world);
        brtire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        const b2RevoluteJointDef brWheelDef = createCustomJoint(body, brtire->GetBody(), b2Vec2{0.5f, -0.75f}, true);
        auto brWheel = b2CreateRevoluteJoint(world_, &brWheelDef);
        tires.push_back(brtire);
        joints.push_back(brWheel);
    }

void Vehicle::update(char control) {
    float maxForwardSpeed = 250;
    float maxBackwardSpeed = -40;
    float forwardSpeed = 0;
    float turn = 0;
    
    if (control == 'w') forwardSpeed = maxForwardSpeed;
    else if (control == 's') forwardSpeed = maxBackwardSpeed;

    if (control == 'a') {
        turn = 1;
        std::cout<<"movingLeft"<<std::endl;
    }
    else if (control == 'd') turn = -1;

    // Apply forward/backward to all tires
    for (int i = 0; i < tires.size(); i++)
        tires[i]->updateFriction();
    for (int i = 0; i < tires.size(); i++) {
        tires[i]->updateDrive(control);
        tires[i]->updateTurn(control);
    }    
    //control steering
    float lockAngle = 35 * DEGTORAD;
    float turnSpeedPerSec = 160 * DEGTORAD;//from lock to lock in 0.5 sec
    float turnPerTimeStep = turnSpeedPerSec / 60.0f;
    float desiredAngle = 0;
    if (control == 'a')  desiredAngle = lockAngle;
    else if (control == 'd') desiredAngle = -lockAngle;
    for (int i = 0; i < 2 && i < joints.size(); i++) {
        auto joint = joints[i];
        float angleNow = b2RevoluteJoint_GetAngle(joint);
        float angleToTurn = desiredAngle - angleNow;
        angleToTurn = Clamp( angleToTurn, -turnPerTimeStep, turnPerTimeStep );
        float newAngle = angleNow + angleToTurn;
        b2RevoluteJoint_SetLimits(joint, newAngle, newAngle);
    }
    b2Vec2 pos = b2Body_GetPosition(body);
    printf("Tire drive: position = (%f, %f), \n",pos.x, pos.y);
}