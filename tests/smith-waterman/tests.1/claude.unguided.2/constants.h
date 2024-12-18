#pragma once

enum scores_e : int
{
  matchScore      = 5,
  missmatchScore = -3,
  gapScore       = -4,
};

enum path_e : int
{
  PATH = -1,
  NONE = 0,
  UP   = 1,
  LEFT = 2,
  DIAGONAL = 3,
};
  
