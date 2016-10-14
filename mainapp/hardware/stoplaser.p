#include "lasergalvo.hp"

.origin 0
.entrypoint SETUP

SETUP:
      ; disable voltage regulator
      clr r30, LASER_CURRENT_ENABLE
      ; disable laser
      clr r30, LASER_ENABLE
      halt