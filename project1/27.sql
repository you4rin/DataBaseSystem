SELECT Trainer.name, MAX(CatchedPokemon.level)
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
GROUP BY Trainer.name
HAVING COUNT(CatchedPokemon.id)>=4
ORDER BY Trainer.name ASC;