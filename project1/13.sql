SELECT Pokemon.name, Pokemon.id
FROM Pokemon
JOIN CatchedPokemon ON CatchedPokemon.pid=Pokemon.id
JOIN Trainer ON CatchedPokemon.owner_id=Trainer.id
WHERE Trainer.hometown='Sangnok City'
ORDER BY CatchedPokemon.pid ASC;