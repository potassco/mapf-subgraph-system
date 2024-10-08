% INFO
% since positions stop at the horizon of the agent
% the reachability for SoC has to take into account the blocked vertices
% that the goals of other agents create
#program time(delta,k).
% set horizon of each agent based on objective
delta_k(delta,k).
%time(A,1..N+delta) :- agent(A), starting_horizon(A,N), k = 0.
newtime(A,0..N+delta,k) :- starting_horizon(A,N), k = 0.
newtime(A,N+PrevDelta+1..N+delta,k) :- agent(A), starting_horizon(A,N), k > 0, delta_k(PrevDelta,k-1).
time(A,T) :- newtime(A,T,k), T > 0.

#program base.
% input starting horizon via API
at(A,P,0) :- start(A,P), agent(R).


#program mapf(k). 
#defined move/4.
#defined time/2.
#defined reach_edge/5.
#defined has_position/3.
% -------------------------------------------------------------------------

% reach(A,V,T,k) are the new reach atoms for the given k, these are given by the application via the backend
% reachk(A,V,T,k) should have all reachable atoms for that particular k and any previous k
reachk(A,V,T) :- reach(A,V,T,k).
%reachk(A,V,T,k) :- reachk(A,V,T,k-1).

% make reach edge for this k, only if they involve the new reach atoms
reach_edge(A,U,V,T,k) :- reach(A,U,T-1,k),    reach(A,V,T,k),    edge(U,V).
reach_edge(A,U,V,T,k) :- reachk(A,U,T-1), not reach(A,U,T-1,k), reach(A,V,T,k),    edge(U,V).
reach_edge(A,U,V,T,k) :- reach(A,U,T-1,k),    reachk(A,V,T), not reach(A,V,T,k), edge(U,V).

reach_edgek(A,U,V,T) :- reach_edge(A,U,V,T,k).
%reach_edgek(A,U,V,T,k) :- reach_edgek(A,U,V,T,k-1).
% no need to consider incoming moves, reachability makes this unnecessary

% these are the outgoing moves
% necessary for the moved_from predicate
% alternatively, moved_from could be defined similarly to the moved predicate (using k)
#external move(A,U,V,T) : agent(A), edge(U,V), not reach_edge(A,U,V,T,k), reach(A,U,T-1,k). [false]

% %incoming moves are needed if a position can come again later from a different place
% this is important since some positions might be blocked by being the goal of agents but when horizon increases the position is not blocked anymore
#external move(A,U,V,T) : agent(A), edge(U,V), not reach_edge(A,U,V,T,k), reach(A,V,T,k). [false]

% choice for possible moves
% at most 1 move per k, later we say at most 1 move per time point (with forward_move predicate)
{ move(A,U,V,T) : reach_edge(A,U,V,T,k)} 1 :- time(A,T).

% moved_from(A,U,T) :- moved_from(A,U,T,k), reach(A,U,T-1,k).
% moved_from(A,U,T,k) :- move(A,U,V,T), reach_edge(A,U,V,T,k).
% moved_from(A,U,T,k) :- moved_from(A,U,T,k+1), reachk(A,U,T-1,k).
% #external moved_from(A,U,T,k+1) : reachk(A,U,T-1,k). [false]

moved_from(A,U,T) :- move(A,U,V,T), reach(A,U,T-1,k).

% % at most 1 move per agent per time point
% moved(A,T,k) :- move(A,U,V,T), reach_edge(A,U,V,T,k).
% forward_move(A,T,k) :- moved(A,T,k).
% forward_move(A,T,k) :- forward_move(A,T,k-1).

% :- forward_move(A,T,k-1), moved(A,T,k).

% - move ------------------------------------------------------------------
% only generate the position at a time point if it is reachable
at(A,V,T) :- move(A,U,V,T), reach(A,V,T,k).

% at(A,V,T) :- at(A,V,T,k), reach(A,V,T,k).
% at(A,V,T,k) :- move(A,U,V,T), reach_edge(A,U,V,T,k).
% at(A,V,T,k) :- at(A,V,T,k+1), reachk(A,V,T,k).
% #external at(A,V,T,k+1) : reachk(A,V,T,k). [false]

% only generate the constraint if the move is possible, that is, the starting position of the move is reachable
:- moved_from(A,U,T), not at(A,U,T-1), reach(A,U,T-1,k).

% - inertia ---------------------------------------------------------------
% at(A,V,T,k) :- at(A,V,T-1), not moved_from(A,V,T), time(A,T), reach(A,V,T,k).
at(A,V,T) :- at(A,V,T-1), not moved_from(A,V,T), time(A,T), reach(A,V,T,k).

% - vertex collision ------------------------------------------------------
% only generate if it can actually happen
% reachk(B,V,T,k) is not necessary, because the "at" atom takes care of having only the possible positions.
% however, if the at literals are generated via externals, it is important to have it.
:- at(A,V,T), at(B,V,T), A != B, time(A,T), time(B,T), reach(A,V,T,k), reachk(B,V,T).

% at most 1 chaining
% :- 2 {at(A,V,T) : reach(A,V,T,k)}, time(_,T), vertex(V).
% has_agent(V,T,k) :- at(A,V,T), reach(A,V,T,k).
% f_has_agent(V,T,k) :- has_agent(V,T,k).
% f_has_agent(V,T,k) :- f_has_agent(V,T,k-1).
% :- f_has_agent(V,T,k-1), has_agent(V,T,k).
% swap conflict
% - edge collision --------------------------------------------------------
% only generate when it can actually happen
% movement(U,V,T) :- movement(U,V,T,k), reach_edge(_,U,V,T,k), not reach_edgek(_,U,V,T,k-1).
% movement(U,V,T,k) :- move(A,U,V,T), reach_edge(A,U,V,T,k).
% movement(U,V,T,k) :- movement(U,V,T,k+1).
% #external movement(U,V,T,k+1) : reach_edgek(_,U,V,T,k). [false]

% :- movement(U,V,T), movement(V,U,T), reach_edge(_,U,V,T,k).
% #show movement/3.
% #show movement/4.
:- move(A,U,V,T), move(B,V,U,T), A != B, time(A,T), time(B,T), reach_edge(A,U,V,T,k), reach_edgek(B,V,U,T), edge(V,U).

% rules for at most one position per agent per time point
% handle multiple position on current k
 :- { at(A,V,T) : reach(A,V,T,k) } > 1, time(A,T).

has_position(A,T,k) :- at(A,V,T), reach(A,V,T,k). 
% handle multiple positions from different k
forward_has_position(A,T,k) :- has_position(A,T,k).
forward_has_position(A,T,k) :- forward_has_position(A,T,k-1).
:- forward_has_position(A,T,k-1), has_position(A,T,k), time(A,T).

% % handle case where no k has position (together with rule in time program)
backward_has_position(A,T,k) :- has_position(A,T,k).
backward_has_position(A,T,k) :- backward_has_position(A,T,k+1).
#external backward_has_position(A,T,k+1): time(A,T). [false]

% this is the lowest k used in the constraint(instead of going down to 0)
#external backward_has_position(A,T,k) : newtime(A,T,k). [false]
:- not backward_has_position(A,T,k), newtime(A,T,k).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#program check(delta,k).
#external query(k-1). [false]
#external query(k). [true]

% can not be somewhere that is not reachable
% maybe enable and disable every time a k is added?
% :- at(A,V,T), not reachk(A,V,T,k), time(A,T), query(k).
% :- goal(A,V), not reachk(A,V,T+delta,k), starting_horizon(A,T), query(k). 
%  - query -----------------------------------------------------------------
:- goal(A,V), not at(A,V,T+delta), starting_horizon(A,T), query(k).

#show.
#show at/3.
% #show reach_edge/5.
#show move/4.
% #show time/2.
% #show reach/4.
#defined reach/4.
% #show moved/3.
% #show forward_move/3.

% #show starting_horizon/2.