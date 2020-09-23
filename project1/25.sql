SELECT DISTINCT Pokemon.name
FROM Pokemon
WHERE Pokemon.id IN(
  SELECT CatchedPokemon.pid
  FROM CatchedPokemon
  JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
  WHERE Trainer.hometown='Sangnok City')
AND Pokemon.id IN(
  SELECT CatchedPokemon.pid
  FROM CatchedPokemon
  JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
  WHERE Trainer.hometown='Brown City')
ORDER BY Pokemon.name ASC;