SELECT Pokemon.name, CatchedPokemon.level, CatchedPokemon.nickname
FROM Pokemon
JOIN CatchedPokemon ON Pokemon.id=CatchedPokemon.pid
JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
JOIN Gym ON Gym.city=Trainer.hometown
WHERE CatchedPokemon.nickname LIKE 'A%'
AND Gym.leader_id=Trainer.id
ORDER BY Pokemon.name DESC;