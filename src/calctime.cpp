// Copyright 1997, 1998, 1999, 2012-2013, 2017-2018 by Jon Dart. All Rights Reserved.
#include "calctime.h"
#include "globals.h"

#include <algorithm>

static const int DEFAULT_MOVES_TO_TC = 35;
static const float PONDER_FACTOR = 1.3F;
static const int GAME_TIME_RESERVE = 75; // try to keep this amt of time in reserve

int calcTimeLimit(int moves, float minutes, int incr,
                  int time_left, int opp_time, bool ponderHit, int trace)
{
    int moves_in_game = gameMoves->num_moves()/2;  // full moves, not half-moves
    int moves_left;
    if (moves == 0) {
        moves_left = 0;
    } else {
        moves_left = moves-(moves_in_game % moves);
    }
    return calcTimeLimit(moves_left,incr,time_left,opp_time,ponderHit,trace);
}


// UCI version
int calcTimeLimit(int movestogo, int incr,
                  int time_left, int opp_time, bool ponderHit, int trace)
{
    if (trace) cout << "# movestogo=" << movestogo << " time_left=" << time_left << endl;
    if (movestogo == 0) movestogo = DEFAULT_MOVES_TO_TC;
    time_left = std::max<int>(time_left-GAME_TIME_RESERVE,0);
    int time_target = (time_left/movestogo) + 9*incr/10 - options.search.move_overhead;

    if (ponderHit && movestogo > 4) {
        time_target = (int)(PONDER_FACTOR*time_target);
    }

    // ensure we never allocate more time than is left
    if (time_target >= time_left + incr) {
        if (trace) {
            cout << "# warning : time_target >= time_left" << endl; 
        }
        time_target = 8*(time_left+incr)/10;
    }

    // enforce minimum search time
    return std::max<int>(time_target,options.search.minimum_search_time);
}
