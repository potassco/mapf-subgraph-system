
#defined horizon/2.
#defined sp_length/2.
#defined delta/1.
#defined time/2.
% set horizon of each agent based on objective
#program sum_of_cost.
horizon(A,H+D) :- agent(A), sp_length(A,H), delta(D).

#program makespan.
horizon(A,H) :- agent(A), mks(H).

#program mapf.
#defined move/4.
time(A,1..T) :- agent(A), horizon(A,T).

% -------------------------------------------------------------------------

at(A,P,0) :- start(A,P), agent(R).

% if using asp to calculate reachability
{ move(A,U,V,T) : edge(U,V), reach(A,V,T)} 1 :- reach(A,U,T-1), time(A,T).

% - move ------------------------------------------------------------------
at(A,V,T) :- move(A,_,V,T).
          :- move(A,U,_,T), not at(A,U,T-1).

% - inertia ---------------------------------------------------------------
at(A,V,T) :- at(A,V,T-1), not move(A,V,_,T), time(A,T), reach(A,V,T).

% - vertex collision ------------------------------------------------------
 :- { at(A,V,T) } > 1, vertex(V), time(_,T).

% swap conflict
% - edge collision --------------------------------------------------------
 :- move(_,U,V,T), move(_,V,U,T), U < V.

% - auxiliaries -------------------------------------------- redundant ----
 :- { at(A,V,T) } != 1, agent(A), time(A,T).
 % - query -----------------------------------------------------------------
:- goal(A,V), not at(A,V,T), horizon(A,T).

#show.
% #show at/3.
% #show move/4.
% #show horizon/2.
% #show sp_length/2.
% #show delta/1.