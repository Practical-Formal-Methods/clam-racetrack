extern void __CRAB_assert(int);
extern void __CRAB_assume(int);

#include <stdio.h>
#include <stdlib.h>

#include "racetrack.h"
#include "safeguard.h"


int main()
{
    /* TODO command line interface */
    Map map = get_map();
    int step_limit = 50;
    int look_ahead_steps = 3;
    int safety_distance = 1;
    State initial_state = get_initial_state(map);
    int success = run_safeguard_controller(map, initial_state, step_limit, look_ahead_steps, safety_distance);

    __CRAB_assert(success == 1);

    return success;
}

