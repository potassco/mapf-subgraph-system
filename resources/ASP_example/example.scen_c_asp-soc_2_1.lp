makespan(8).
makespan(1,4).
makespan(2,8).
delta(0).
soc_lb(12).
vertex((1,2)). vertex((1,4)). vertex((2,1)). vertex((2,2)). vertex((2,3)). vertex((2,4)). vertex((2,5)). vertex((2,6)). vertex((2,7)). vertex((2,8)). vertex((3,1)). 
edge((1,2),(2,2)). edge((1,4),(2,4)). edge((2,1),(3,1)). edge((2,1),(2,2)). edge((2,2),(1,2)). edge((2,2),(2,1)). edge((2,2),(2,3)). edge((2,3),(2,2)). edge((2,3),(2,4)). edge((2,4),(1,4)). edge((2,4),(2,3)). edge((2,4),(2,5)). edge((2,5),(2,4)). edge((2,5),(2,6)). edge((2,6),(2,5)). edge((2,6),(2,7)). edge((2,7),(2,6)). edge((2,7),(2,8)). edge((2,8),(2,7)). edge((3,1),(2,1)). 
start(1,(1,4)). start(2,(3,1)). 
goal(1,(1,2)). goal(2,(2,8)). 
agent(1).agent(2).
poss_loc(1,(1,2),4..8). poss_loc(1,(1,4),0). poss_loc(2,(2,1),1). poss_loc(1,(2,2),3). poss_loc(2,(2,2),2). poss_loc(1,(2,3),2). poss_loc(2,(2,3),3). poss_loc(1,(2,4),1). poss_loc(2,(2,4),4). poss_loc(2,(2,5),5). poss_loc(2,(2,6),6). poss_loc(2,(2,7),7). poss_loc(2,(2,8),8). poss_loc(2,(3,1),0). 
