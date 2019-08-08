# GAA_Game

 - Game Structure/Design
 
Time Class - Controls the game time and duration
  GenerateMatchClock()
  StartTime()
  HalfTime()
  Full-Time()
  Extra-Time()
  PenaltyTime()
  
Game Class - Starts match competition
  Team Class -
    TeamStructure()
    TeamInformation()
    TeamQuality()
    
    Player Class -
      NumberOfPlayers()
      PlayerNames()
      PlayerHealth()
      PlayerFitness()
      
    Level Class -
      PlayerXP()
      PlayerLevel()
      
  Attack Class -
    Attack();
    AttackResult()
    
  Score Class -
    DisplayScore()
    UpdateScore()
    
  Stats Class -
    RecordTeamGoals()
    RecordTeamPoints()
    RecordTeamMisses()
    RecordTeamFouls()
      RecordPlayerGoals()
      RecordPlayerPoints()
      RecordPlayerMisses()
      RecordPlayerFouls()
      
  Reports Class -
    DisplayPlayerFouls()
    DisplayPlayerInjuries()
    DisplayPlayerScoreStreaks()
    
  Results Class -
    HalfTimeResult()
    FullTimeResult()
