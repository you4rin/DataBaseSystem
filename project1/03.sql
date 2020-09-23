SELECT AVG(CatchedPokemon.level)
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
JOIN Pokemon ON CatchedPokemon.pid=Pokemon.id
WHERE Pokemon.type='Electric'
AND Trainer.hometown='Sangnok City';