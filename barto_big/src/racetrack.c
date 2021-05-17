extern int __CRAB_nd();
extern void __CRAB_assert(int);
extern void __CRAB_assume(int);
extern void __CRAB_intrinsic_get_wall_distance(int, int, int, int, int);
extern void __CRAB_intrinsic_is_valid_velocity(int, int, int, int, int);

#include <stdio.h>
#include <stdlib.h>

#include "racetrack.h"

int is_null_state(State state)
{
    if(state.position_x == -100 && state.position_y == -100 && state.velocity_x == -100 && state.velocity_y == -100)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

State simulate_acceleration(const Map map, const State state, const Acceleration acceleration)
{
    return get_next_state(map, state, acceleration);
}

State execute_acceleration(const Map map, const State state, const Acceleration acceleration)
{
    return get_next_state(map, state, acceleration);
}

State get_initial_state(const Map map)
{
    State initial_state = {0, 14, 0, 0};

    return initial_state;
}

State get_next_state(const Map map, const State state, const Acceleration acceleration)
{
    if (!is_valid_acceleration(map, state, acceleration))
    {
        State null_state = {-100, -100, -100, -100}; //Arbitary choice
        return null_state;
    }

    Velocity next_velocity = {state.velocity_x + acceleration.x, state.velocity_y + acceleration.y};
    Position next_position = {state.position_x + next_velocity.x, state.position_y + next_velocity.y};
    State next_state = {next_position.x, next_position.y, next_velocity.x, next_velocity.y};
    return next_state;
}

int is_goal_state(const Map map, const State state)
{
    Position position = {state.position_x, state.position_y};
    Velocity velocity = {state.velocity_x, state.velocity_y};
    return is_goal(map, position) && is_zero(velocity);
}

Acceleration get_negated_acceleration(const Acceleration acceleration)
{
    Acceleration negated_acceleration = {-acceleration.x, -acceleration.y};
    return negated_acceleration;
}

Position get_position(const State state)
{
    Position p = {state.position_x, state.position_y};
    return p;
}

Velocity get_velocity(const State state)
{
    Velocity v = {state.velocity_x, state.velocity_y};
    return v;
}

Acceleration create_acceleration(int x, int y)
{
    Acceleration acceleration = {x, y};
    return acceleration;
}

int is_valid_acceleration(const Map map, const State state, const Acceleration acceleration)
{
    Velocity accumulated_velocity = {state.velocity_x + acceleration.x, state.velocity_y + acceleration.y};
    int is_valid = __CRAB_nd();
    __CRAB_intrinsic_is_valid_velocity(state.position_x, state.position_y, accumulated_velocity.x, accumulated_velocity.y, is_valid);
    return is_valid;
}

int is_valid_position(const Position position)
{
    int map_pos = 0;
    if((position.x >= 0)&&(position.x <14) && (position.y>=0) && (position.y <14))
    {
       map_pos = 0;
    }
    else if(position.x == 24) 
    {
       map_pos = 0;
    }
    else if(position.y == 24)
    {
        map_pos = 0;
    }
    else if(position.x < 0)
    {
       map_pos = 0;
    }
    else if (position.x > 24)
    {
       map_pos = 0;
    }
    else if(position.y < 0)
    {
       map_pos = 0;
    }
    else if (position.y > 24)
    {
       map_pos = 0;
    }
    else 
    {
       map_pos = 1;
    }

    return map_pos;
}

int is_goal(const Map map, const Position position)
{
    int x = position.x;
    int y = position.y;
    if(y == 0 && x>=14 && x < 24)
       return 1;
    else
       return 0;
}

int is_zero(const Velocity velocity)
{
    int vx = velocity.x;
    int vy = velocity.y;
    return vx == 0 && vy == 0;
}

Position get_start_position(const Map map)
{
    Position start_position = {0, 14};
    return start_position;
}

Velocity get_start_velocity()
{
    Velocity start_velocity = {0, 0};
    return start_velocity;
}

void delete_map(Map *map)
{
    free(map);
}

void delete_position(Position *position)
{
    free(position);
}

void delete_velocity(Velocity *velocity)
{
    free(velocity);
}

void delete_acceleration(Acceleration *acceleration)
{
    free(acceleration);
}

void delete_distance(Distance *distance)
{
    free(distance);
}

void delete_state(State *state)
{
    free(state);
}

Map get_map()
{
    Map map = {25, 25, 10, 10}; //For our fixed Map

    return map;
}
