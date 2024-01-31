#program optimize.
#defined cost/1.
#defined penalty/2.
penalty(A,N) :- sp_length(A,N+1), N >= 0.
penalty(A,T) :- sp_length(A,N), at(A,U,T), not goal(A,U), T >= N.
penalty(A,T) :- penalty(A,T+1), T >= 0.

bound(H+D) :- H = #sum{T,A : sp_length(A,T)}, delta(D).

:- #sum{ 1,A,T : penalty(A,T) } > B, bound(B).

#show penalty/2.