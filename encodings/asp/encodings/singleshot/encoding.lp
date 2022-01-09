% = Combined Encoding for non-anonymous MAPF with possible locations  ==============================

% Move actions
#include "./../asprilo/m/action-M.lp".

% Enforce assigned goal locations
#include "./assigned-goal.lp".

% Enforce possible locations
#include "./possible_location.lp".

% Output conversion
#include "./../asprilo/m/output-M.lp".
