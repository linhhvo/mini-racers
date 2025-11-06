#include "CheckpointManager.hpp"

CheckpointManager::CheckpointManager(b2WorldId world, int totalLaps, int numCars)
    : world_(world), totalLaps_(totalLaps)
{
    carProgress_.resize(numCars); // initialize progress for all cars
}

CheckpointManager::~CheckpointManager() {
    for (auto c : checkpoints_) {
        delete c;
    }
}

void CheckpointManager::AddCheckpoint(const b2Vec2& position) {
    Checkpoint* cp = new Checkpoint(world_, position);
    checkpoints_.push_back(cp);
}

void CheckpointManager::HandleSensorEvent(b2SensorBeginTouchEvent* e) {
    void* sensorData = b2Shape_GetUserData(e->sensorShapeId);
    void* visitorData = b2Shape_GetUserData(e->visitorShapeId);

    // Find which checkpoint was triggered
    int checkpointIndex = -1;
    for (size_t i = 0; i < checkpoints_.size(); i++) {
        if (B2_ID_EQUALS(checkpoints_[i]->GetShapeId(), e->sensorShapeId)) { 
            checkpointIndex = i;
            break;
        }
    }

    if (checkpointIndex == -1) return; // safety check

    // Find which car triggered it
    int carID = -1;
    for (size_t i = 0; i < carProgress_.size(); i++) {
        void* carShapeData = visitorData; // assuming visitorData points to the car object
        if (carShapeData == visitorData) { // modify as needed for your car system
            carID = i;
            break;
        }
    }
    if (carID == -1) return; // car not found

    CarProgress& progress = carProgress_[carID];

    // Only count if the checkpoint is next in order
    if ((int)checkpointIndex == progress.nextCheckpointIndex) {
        progress.nextCheckpointIndex++;

        // Completed lap?
        if (progress.nextCheckpointIndex >= (int)checkpoints_.size()) {
            progress.currentLap++;
            progress.nextCheckpointIndex = 0;

            std::cout << "Car " << carID << " completed lap " << progress.currentLap << "!\n";

            // Finished race?
            if (progress.currentLap >= totalLaps_ && !raceFinished) {
                progress.finished = true;
                raceFinished = true;
                winnerCarID = carID;
                std::cout << "Car " << carID << " wins the race!\n";
            }
        }
    }

}
