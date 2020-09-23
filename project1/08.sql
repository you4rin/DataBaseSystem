SELECT AVG(CatchedPokemon.level)
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
WHERE Trainer.name='Red';