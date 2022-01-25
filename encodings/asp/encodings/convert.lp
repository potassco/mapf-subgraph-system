% = Input Conversion from movingai to asprilo fact format ==========================================

#show.

% - Robots -----------------------------------------------------------------------------------------

% -- Start locations -------------------------------------------------------------------------------
#show init(object(robot,R),       value(at,(X,Y)))  : start(   R, X,Y).

% -- Goal locations --------------------------------------------------------------------------------

% * init/2 format in Md domain:
% #show init(object(destination,R), value(at,(X,Y)))  : goal(    R, X,Y).

% * internal format:
#show goal(robot(R),(X,Y))                          : goal(    R, X,Y).

% * for visualization as shelf
#show init(object(shelf,R),       value(at,(X,Y)))  : goal(    R, X,Y).

% -- Possible positions, optional ------------------------------------------------------------------
#defined poss_loc/3.

% * init/2 format suggestion
% #show init(object(possLoc,R),     value(at,(X,Y)))  : poss_loc(R,(X,Y),T).

% * internal format
#show poss_loc(robot(R),(X,Y),T)                    : poss_loc(R,(X,Y),T).


% - Grid nodes  ------------------------------------------------------------------------------------
#show init(object(node,(X,Y)),value(at,(X,Y)))      : node(       X,Y).


% - Horizon ----------------------------------------------------------------------------------------
#show horizon(H) : makespan(H).
