agent_SP(A,H-D) :- makespan(A,H), delta(D).

at_goal_back(A,M) :- agent(A), makespan(M).
at_goal_back(A,T-1) :- at_goal_back(A,T), wait(A,T), agent_SP(A,H), T>=H.

wait(A,T) :- not move(A,_,_,T), agent(A), time(T), agent_SP(A,H), T>=H.

cost(A,T) :- at(A,V,T-1), not goal(A,V), time(T), agent_SP(A,H), T>=H.
cost(A,T) :- at(A,V,T-1), goal(A,V), not wait(A,T), time(T), agent_SP(A,H), T>=H.
cost(A,T) :- at(A,V,T-1), goal(A,V), wait(A,T), not at_goal_back(A,T-1), time(T), agent_SP(A,H), T>=H.


% all moves below their SP have a cost
cost(A,1..T) :- agent_SP(A,T).

bound(H+D) :- soc_lb(H), delta(D).

:- cost(S), S > B, bound(B).

%cost(S) :- S = #sum{ 1,A,T : cost(A,T) ; H-1,A : agent_horizon(A,H) }.
cost(S) :- S = #sum{ 1,A,T : cost(A,T)}.
%#show wait/2.
%#show at_goal_back/2.
%#show move/4.

#show cost/1.