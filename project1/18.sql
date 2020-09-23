SELECT AVG(CatchedPokemon.level)
FROM CatchedPokemon
JOIN Trainer ON Trainer.id=CatchedPokemon.owner_id
JOIN Gym ON Gym.leader_id=Trainer.id;