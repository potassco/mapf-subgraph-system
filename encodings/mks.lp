time(1..M) :- makespan(M).

used_vertex(V) :- poss_loc(_,V,_).
used_edge(U,V) :- poss_loc(_,U,_), poss_loc(_,V,_), edge(U,V).

direction((X,Y)) :- X=-1..1, Y=-1..1, |X+Y|=1.
nextto((X,Y),(DX,DY),(X',Y')) :- direction((DX,DY)), used_vertex((X,Y)), used_vertex((X',Y')),
                                 (X,Y)=(X'-DX,Y'-DY), (X',Y')=(X+DX,Y+DY), used_edge((X,Y), (X',Y')).

at(R,C,0) :- start(R,C).

{ move(R,D,T) : direction(D) } 1 :- agent(R), time(T).
%wait(A,T) :- not move(A,_,T), agent(A), time(T), agent_SP(A,H), T>=H.

% - move/3 ----------------------------------------------------------------------
at(R,C,T) :- move(R,D,T), at(R,C',T-1),     nextto(C',D,C), used_edge(C',C), poss_loc(R,C,T).
                :- move(R,D,T), at(R,C ,T-1), not nextto(C ,D,_), poss_loc(R,C,T-1).

% - inertia ---------------------------------------------------------------------
at(R,C,T) :- at(R,C,T-1), not move(R,_,T), agent(R), time(T).

% - edge collision --------------------------------------------------------------
moveto(C',C,T) :- nextto(C',D,C), at(R,C',T-1), move(R,D,T), used_edge(C',C), poss_loc(R,C',T-1), poss_loc(R,C,T).
 :- moveto(C',C,T), moveto(C,C',T), C < C', used_edge(C',C).

% - vertex collision ------------------------------------------------------------
 :- { at(R,C,T) : agent(R) }  > 1, used_vertex(C), time(T).

% - auxiliaries -----------------------------------------------------------------
 :- { at(R,C,T) } != 1, agent(R), time(T).    % REDUNDANT but PERFORMANT?

 :- not at(R,C,M), goal(R,C), makespan(M).

:- not poss_loc(R,C,T), at(R,C,T), agent(R), time(T).

#show.