#defined makespan/2.
#defined spath/4.
at(R,P,0) :- start(R,P).
% if spath exists, get agent_SP from that, otherwise, assume it comes from somewhere else
agent_SP(A,H) :- agent(A), H = #max{ T : spath(A,_,_,T) ; 0 }, spath(_,_,_,_).
agent_horizon(A,H+D) :- agent(A), agent_SP(A,H), not makespan(A,_), delta(D).
agent_horizon(A,H) :- agent(A), makespan(A,H).
agent_time(A,1..T) :- agent(A), agent_horizon(A,T).

% calculate edges for each agent given the sum of cost delta

#const agent_edges=-3.
% base case, every edge available to every agent up to to the overall horizon
edge(A,U,V,T) :- edge(U,V), agent(A), time(T), agent_edges=0.
agent_time(A,1..M) :- makespan(M), agent(A), agent_edges=0.

% base case, every edge available to every agent up to their horizon
edge(A,U,V,T) :- edge(U,V), agent_time(A,T), agent_edges=1.

%%% base case for makespan, reachability goes to the overall horizon
%%%mostly for comparison purposes
agent_time(A,1..M) :- makespan(M), agent(A), agent_edges=6.
% forward edges
freach(A,U,0) :- at(A,U,0), agent_edges=6.
freach(A,U,T) :- freach(A,U,T-1), time(T), agent_edges=6.
freach(A,V,T) :- freach(A,U,T-1), edge(U,V), time(T), agent_edges=6.

% backward edges
breach(A,U,M) :- goal(A,U), makespan(M), agent_edges=6.
breach(A,U,T-1) :- breach(A,U,T), time(T), agent_edges=6.
breach(A,U,T-1) :- breach(A,V,T), edge(U,V), time(T), agent_edges=6.

reach(A,V,T) :- freach(A,V,T), breach(A,V,T), agent_edges=6.
:- at(A,V,T), not reach(A,V,T), agent_time(A,T), agent_edges=6.

edge(A,U,V,T) :- edge(U,V), reach(A,V,T), reach(A,U,T-1), agent_edges=6.

%%%%%%%%%%%%%%
% reachability up to horizon of the given agent + blocks based on goals of other agents

%blocked(A,U,V,T) :- agent_time(A,T), agent_horizon(B,H), edge(U,V), goal(B,V), T >= H, A != B, agent_edges=7.
%blocked(A,U,V,T) :- agent_time(A,T), agent_horizon(B,H), edge(U,V), goal(B,U), T > H, A != B, agent_edges=7.

% this means you can not go into this vertex at this time point
fblock(A,V,T) :- agent_time(A,T), agent_horizon(B,H), goal(B,V), T >= H, A != B, agent_edges=7. 
% this means you can not go out of this vertex at this time point.
bblock(A,V,T) :- agent_time(A,T), agent_horizon(B,H), goal(B,V), T > H, A != B, agent_edges=7.
%bblock(A,V,T) :- fblock(A,V,T), fblock(A,V,T-1).

freach(A,U,0) :- at(A,U,0), agent_edges=7.
freach(A,U,T) :- freach(A,U,T-1), not fblock(A,U,T), agent_time(A,T), agent_edges=7.
freach(A,V,T) :- freach(A,U,T-1), edge(U,V), not fblock(A,V,T), agent_time(A,T), agent_edges=7.

breach(A,V,H) :- goal(A,V), agent_horizon(A,H), agent_edges=7.
breach(A,U,T-1) :- breach(A,U,T), not bblock(A,U,T-1), agent_time(A,T), agent_edges=7.
breach(A,U,T-1) :- breach(A,V,T), edge(U,V), not bblock(A,U,T-1), agent_time(A,T), agent_edges=7.

reach(A,V,T) :- freach(A,V,T), breach(A,V,T), agent_edges=7.
% This constraint works because edges only say possible movements not including waits
% since the wait is just not moving, wait is possible at any point even if the vertex it waited on
% is not reachable in the next state
% this constraint avoids those cases
:- at(A,V,T), not reach(A,V,T), agent_time(A,T), agent_edges=7.

edge(A,U,V,T) :- edge(U,V), reach(A,V,T), reach(A,U,T-1), agent_edges=7.

% just for comparison
edgec(C) :- C = #count{A,U,V,T : edge(A,U,V,T)}.
#show edgec/1.
%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% from here on we use the poss locs
%%%%%%%%%%%%%%%%%%%%%%%%%%%
#defined poss_loc/3.

% combine agent_edges -1 and -2
:- at(A,U,T), not poss_loc(A,U,T), agent_time(A,T), agent_edges=-3.
vertex(A,V) :- poss_loc(A,V,_), agent_edges=-3.

%%%%%%%%%%%%%%%%%%%%%%%%%%%

% agent vertices
vertex(A,V) :- edge(A,V,_,_), agent_edges >=0.
vertex(A,V) :- edge(A,_,V,_), agent_edges >=0.
% -------------------------------------------------------------------------
%#const horizon=0.
%time(1..horizon).
time(1..M) :- makespan(M).
#const alt=0.
% if using asp to calculate reachability
{ move(R,U,V,T) : edge(R,U,V,T)} 1 :- agent(R), agent_time(R,T), agent_edges>=0, alt=0.
{ move(R,U,V,T) : edge(U,V), reach(A,V,T), reach(A,U,T-1)} 1 :- agent(R), agent_time(R,T), agent_edges>=0, alt=1.
% this one if using the poss locs

{ move(R,U,V,T) : edge(U,V), poss_loc(R,V,T), poss_loc(R,U,T-1)} 1 :- agent(R), agent_time(R,T), agent_edges=-3.


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
%:- goal(R,V), not at(R,V,horizon).
:- goal(R,V), not at(R,V,M), makespan(M).

#show.
%#show at/3.
%#show move/4.
