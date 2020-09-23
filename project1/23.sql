SELECT DISTINCT Trainer.name
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
WHERE CatchedPokemon.level<=10
ORDER BY Trainer.name ASC;