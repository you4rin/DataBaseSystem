SELECT Trainer.name,COUNT(CatchedPokemon.id)
FROM Trainer
JOIN Gym ON Trainer.hometown=Gym.city
JOIN CatchedPokemon ON Trainer.id=CatchedPokemon.owner_id
WHERE Trainer.id=Gym.leader_id
GROUP BY Trainer.name
ORDER BY Trainer.name ASC;