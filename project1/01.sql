SELECT Trainer.name
FROM Trainer, CatchedPokemon
WHERE Trainer.id = CatchedPokemon.owner_id
GROUP BY CatchedPokemon.owner_id
HAVING COUNT(CatchedPokemon.id) >= 3 
ORDER BY COUNT(CatchedPokemon.id) DESC;