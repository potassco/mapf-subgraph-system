#program optimize(delta,k).
#external opt(k-1). [false]
#external opt(k). [true]

#defined cost/1.
#defined penalty/2.
#defined penalty/3.
#defined starting_horizon/2.

penalty(A,0..N-1) :- starting_horizon(A,N), k=0.

penalty(A,T,k) :- starting_horizon(A,N), T>=N, at(A,U,T), not goal(A,U), reach(A,U,T,k).

#external penalty(A,T,k+1) : starting_horizon(A,N), time(A,T), T>=N. [false]
penalty(A,T,k) :- penalty(A,T,k+1).

% for new timepoints which have penalties, introduce the way to get the base penalty
penalty(A,T) :- penalty(A,T,k), newtime(A,T,k), starting_horizon(A,N), T<N+delta.

% for the last time point, no penalty(A,T,k) is generated because the only reach atom is to the goal
% so we need to generate a penalty for the last time point of the given k
% that becomes true from a penalty in the next k
penalty(A,N+delta) :- penalty(A,N+delta,k+1), starting_horizon(A,N), newtime(A,_,k).

#external penalty(A,N+delta+1) : starting_horizon(A,N), newtime(A,_,k). [false]
penalty(A,T) :- penalty(A,T+1), newtime(A,T,k), starting_horizon(A,N).

bound(H) :- H = #sum{T,A : starting_horizon(A,T)}, k=0.

:- #sum{ 1,A,T : penalty(A,T) } > B+delta, bound(B), opt(k).

#show penalty/2.
#show penalty/3.
#show starting_horizon/2.
