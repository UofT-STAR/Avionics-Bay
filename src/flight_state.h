#ifndef FLIGHT_STATE_H
#define FLIGHT_STATE_H

enum RocketState { PRE_LAUNCH, ASCENT, DESCENT, LAND };

struct StateUpdateResult {
  bool launched;
  bool deployTriggered;
  bool landed;
};

StateUpdateResult updateRocketState(
  float currAlt,
  float currAcc,
  RocketState &currentState,
  float &maxAlt,
  int &counter,
  float verticalGThreshold
);

#endif