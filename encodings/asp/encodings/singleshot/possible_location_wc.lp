% = If present, use poss_loc/3 facts to constraint possible robot positions on the grid ============
#defined poss_loc/3.
err_poss_loc(R,C,T) :- position(robot(R),C,T), not poss_loc(R,C,T), poss_loc(_,_,_).

#minimize{ R,C,T : err_poss_loc(R,C,T) }.

#show err_poss_loc/3.
