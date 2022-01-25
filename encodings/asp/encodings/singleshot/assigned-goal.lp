% - Enforce non-anonymous MAPF, i.e., each robot must reach its assigned goal grid node ------------
:- not position(R,C,horizon), goal(R,C).
