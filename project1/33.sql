SELECT SUM(CatchedPokemon.level)
FROM Trainer
JOIN CatchedPokemon ON Trainer.id=CatchedPokemon.owner_id
WHERE Trainer.name='Matis'
GROUP BY Trainer.name;