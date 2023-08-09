#defined makespan/2.
at(R,P,0) :- start(R,P), agent(R).

% if spath exists, get agent_SP from that, otherwise, assume it comes from somewhere else
agent_time(A,1..H) :- agent(A), makespan(A,H).
agent_time(A,1..M) :- agent(A), makespan(M), not makespan(A,_).
%agent_time(A,1..T) :- agent(A), agent_horizon(A,T).

% calculate edges for each agent given the sum of cost delta

%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% from here on we use the poss locs
%%%%%%%%%%%%%%%%%%%%%%%%%%%
#defined poss_loc/3.
agent_time(A,T) :- poss_loc(A,_,T).
% combine agent_edges -1 and -2
:- at(A,U,T), not poss_loc(A,U,T), agent_time(A,T).

vertex(A,V) :- poss_loc(A,V,_).


% -------------------------------------------------------------------------
%#const horizon=0.
%time(1..horizon).
time(1..M) :- makespan(M).

{ move(R,U,V,T) : edge(U,V), poss_loc(R,V,T), poss_loc(R,U,T-1)} 1 :- agent(R), agent_time(R,T).

% - move ------------------------------------------------------------------
at(R,V,T) :- move(R,_,V,T).
          :- move(R,U,_,T), not at(R,U,T-1).

% - inertia ---------------------------------------------------------------
at(R,V,T) :- at(R,V,T-1), not move(R,V,_,T), time(T).

% - vertex collision ------------------------------------------------------
 :- { at(R,V,T) : agent(R), vertex(R,V) }  > 1, vertex(V), time(T).

% swap conflict
% - edge collision --------------------------------------------------------
 :- move(_,U,V,T), move(_,V,U,T), U < V.

% - auxiliaries -------------------------------------------- redundant ----
 :- { at(R,V,T) } != 1, agent(R), time(T).

 % - query -----------------------------------------------------------------
:- goal(R,V), not at(R,V,M), makespan(M), agent(R).

#show.
%#show at/3.
%#show move/4.
