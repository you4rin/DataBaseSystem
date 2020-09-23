SELECT Trainer.name
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
GROUP BY Trainer.name
HAVING COUNT(DISTINCT CatchedPokemon.pid)<>COUNT(CatchedPokemon.id)
ORDER BY Trainer.name;