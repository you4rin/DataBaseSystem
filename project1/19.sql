SELECT COUNT(DISTINCT Pokemon.type)
FROM Pokemon
JOIN CatchedPokemon ON Pokemon.id=CatchedPokemon.pid
JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
JOIN Gym ON Trainer.hometown=Gym.city
WHERE Gym.leader_id=Trainer.id
AND Trainer.hometown='Sangnok City';