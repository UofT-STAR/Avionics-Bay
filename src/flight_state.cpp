#include "flight_state.h"

StateUpdateResult updateRocketState(
  float currAlt,
  float currAcc,
  RocketState &currentState,
  float &maxAlt,
  int &counter,
  float verticalGThreshold
) {
  StateUpdateResult result = { false, false, false };

  switch (currentState) {
    case PRE_LAUNCH:
      if (currAlt > 40.0f || (currAlt > 20.0f && currAcc > verticalGThreshold) || currAcc > 5.0f) {
        counter++;
      } else {
        counter = 0;
      }

      if (counter >= 10) {
        counter = 0;
        currentState = ASCENT;
        result.launched = true;
      }
      break;

    case ASCENT:
      if (currAlt > maxAlt) {
        maxAlt = currAlt;
        counter = 0;
      }

      if (currAlt < maxAlt - 10.0f || currAcc < -1.0f) {
        counter++;
      }

      if (counter >= 10) {
        counter = 0;
        currentState = DESCENT;
        result.deployTriggered = true;
      }
      break;

    case DESCENT:
      if (currAlt <= 10.0f) {
        counter++;
      }

      if (counter >= 20) {
        counter = 0;
        currentState = LAND;
        result.landed = true;
      }
      break;

    case LAND:
      break;
  }

  return result;
}