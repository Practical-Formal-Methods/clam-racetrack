extern int __CRAB_nd();
extern void __CRAB_assert(int);
extern void __CRAB_assume(int);
extern void __CRAB_intrinsic_get_wall_distance(int, int, int, int, int);
extern void __CRAB_intrinsic_get_goal_distance(int, int, int, int);
extern void __CRAB_intrinsic_deepsymbol(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

#include "stddef.h"
#include "safeguard.h"
#include "racetrack.h"

Acceleration compute_acceleration(const Map map, const State state, int look_ahead_steps, int safety_distance);

int look_ahead_check(const Map map, const State state, int look_ahead_steps, int safety_distance);

/* TODO implement safety distance */
int run_safeguard_controller(const Map map, const State initial_state, int step_limit, int look_ahead_steps, int safety_distance)
{

    if (step_limit < 1)
    {
        return 0;
    }

    int goal_check = is_goal_state(map, initial_state);

    if (goal_check)
    {
        return 1;
    }

    /* step zero */
    int step = 0;
    Acceleration acceleration;
    State state, next_state;

    acceleration = compute_acceleration(map, initial_state, look_ahead_steps, safety_distance);
    state = execute_acceleration(map, initial_state, acceleration);

    for(step = 1; step < step_limit; step++){
        
        if(is_goal_state(map, state)){
            return 1;
        }
        else if(is_null_state(state)){
            return 0;
        }
        else{
            acceleration = compute_acceleration(map, state, look_ahead_steps, safety_distance);
            next_state = execute_acceleration(map, state, acceleration);
            state = next_state;
        }
    }

    return 1;
    
}

Acceleration compute_acceleration(const Map map, const State state, int look_ahead_steps, int safety_distance){

    Acceleration acceleration;
    acceleration.x = __CRAB_nd();
    acceleration.y = __CRAB_nd();
    acceleration = call_nn_model(map, state);

    if (!look_ahead_check(map, state, look_ahead_steps, safety_distance))
    {
        Acceleration negated_acceleration = get_negated_acceleration(acceleration);
        return negated_acceleration;
    }
    else{
        return acceleration;
    }
}

int look_ahead_check(const Map map, const State state, int look_ahead_steps, int safety_distance)
{

    for(int i=0;i <look_ahead_steps; i++){

        Acceleration simulated_acceleration = call_nn_model(map, state);
        State simulated_state = simulate_acceleration(map, state, simulated_acceleration);
        
        if (is_null_state(simulated_state))
        {
            return 0;
        }
    }

    return 1;
}

Acceleration call_nn_model(const Map map, const State state)
{
    // First we prepare the input to the NN here
    // Get fresh feature variables
    int f1 = __CRAB_nd(), f2 = __CRAB_nd(), f3 = __CRAB_nd(), f4 = __CRAB_nd(),
        f5 = __CRAB_nd(), f6 = __CRAB_nd(), f7 = __CRAB_nd(), f8 = __CRAB_nd(),
        f9 = __CRAB_nd(), f10 = __CRAB_nd(), f11 = __CRAB_nd(), f12 = __CRAB_nd(),
        f13 = __CRAB_nd(), f14 = __CRAB_nd();
    f1 = state.position_x;
    f2 = state.position_y;
    f3 = state.velocity_x;
    f4 = state.velocity_y;
    
    Position position = {state.position_x, state.position_y};

    __CRAB_intrinsic_get_wall_distance(position.x, position.y, -1, -1, f5); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, -1, 0, f6); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, -1, 1, f7); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, 0, -1, f8); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, 0, 1, f9); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, 1, -1, f10); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, 1, 0, f11); 
    __CRAB_intrinsic_get_wall_distance(position.x, position.y, 1, 1, f12); 

    __CRAB_intrinsic_get_goal_distance(position.x, position.y, f13, f14);
    
    int ax = __CRAB_nd();
    int ay = __CRAB_nd();

    __CRAB_intrinsic_deepsymbol(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, ax, ay);

    Acceleration acceleration = create_acceleration(ax, ay);

    return acceleration;

}
