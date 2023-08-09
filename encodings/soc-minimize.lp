#const jump=0.
agent_SP(A,H-D) :- makespan(A,H), delta(D), jump=0.

at_goal_back(A,M) :- agent(A), makespan(M).
at_goal_back(A,T-1) :- at_goal_back(A,T), wait(A,T), agent_SP(A,H), T>=H.

wait(A,T) :- not move(A,_,_,T), agent(A), time(T), agent_SP(A,H), T>=H.

cost(A,T) :- at(A,V,T-1), not goal(A,V), time(T), agent_SP(A,H), T>=H.
cost(A,T) :- at(A,V,T-1), goal(A,V), not wait(A,T), time(T), agent_SP(A,H), T>=H.
cost(A,T) :- at(A,V,T-1), goal(A,V), wait(A,T), not at_goal_back(A,T-1), time(T), agent_SP(A,H), T>=H.


% all moves below their SP have a cost
cost(A,1..T) :- agent_SP(A,T).
cost(S) :- S = #sum{ 1,A,T : cost(A,T)}.

#minimize{1,A,T : cost(A,T)}.

#show cost/1.
#show delta/1.
#show makespan/1.
#show makespan/2.
#show agent_SP/2.
#show soc_lb/1.