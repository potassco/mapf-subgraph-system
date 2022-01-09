% = If present, use poss_loc/3 facts to constraint possible robot positions on the grid ============
#defined poss_loc/3.
:- not poss_loc(R,C,T), position(R,C,T), isRobot(R), poss_loc(_,_,_).
