SELECT COUNT(DISTINCT CatchedPokemon.pid)
FROM CatchedPokemon
JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
WHERE Trainer.hometown='Sangnok City';