#program optimize.
#defined penalty/2.
#defined cost/1.
penalty(A,N) :- sp_length(A,N+1), N >= 0.
penalty(A,T) :- sp_length(A,N), at(A,U,T), not goal(A,U), T >= N.
penalty(A,T) :- penalty(A,T+1), T >= 0.

#minimize{1,A,T : penalty(A,T)}.

#show penalty/2.