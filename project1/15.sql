SELECT Trainer.id, COUNT(CatchedPokemon.id)
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
GROUP BY Trainer.id
HAVING COUNT(CatchedPokemon.id) IN(
  SELECT MAX(id_count)
  FROM (
    SELECT COUNT(CatchedPokemon.id) AS id_count
    FROM Trainer
    JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
    GROUP BY Trainer.name
    )CNT
  )
ORDER BY Trainer.id ASC;