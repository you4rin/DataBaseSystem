SELECT Trainer.name, AVG(CatchedPokemon.level)
FROM Trainer
JOIN CatchedPokemon ON CatchedPokemon.owner_id=Trainer.id
JOIN Gym ON Gym.city=Trainer.hometown
WHERE Trainer.id=Gym.leader_id
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;