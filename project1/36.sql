SELECT Trainer.name
FROM Trainer
JOIN CatchedPokemon x ON x.owner_id=Trainer.id
WHERE x.pid NOT IN(
  SELECT Pokemon.id 
  FROM Pokemon
  JOIN CatchedPokemon y ON y.pid=Pokemon.id
  JOIN Evolution ON y.pid=Evolution.before_id
  )
AND x.pid IN(
  SELECT Pokemon.id 
  FROM Pokemon
  JOIN CatchedPokemon z ON z.pid=Pokemon.id
  JOIN Evolution ON z.pid=Evolution.after_id
  )
ORDER BY Trainer.name ASC;