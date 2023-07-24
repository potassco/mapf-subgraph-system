vertex((X,Y)) :- node(X,Y).
edge((X,Y),(X',Y')) :- vertex((X,Y)), vertex((X',Y')), |X-X'| + |Y-Y'| = 1.
agent(R) :- start(R,_,_).
start(R,(X,Y)) :- start(R,X,Y).
goal(R,(X,Y)) :- goal(R,X,Y).

%#show vertex/1.
%#show edge/2.
%#show agent/1.
%#show goal/2.
%#show start/2.
%#show spath/4.
