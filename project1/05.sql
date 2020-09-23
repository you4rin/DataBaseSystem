SELECT Trainer.name
FROM Trainer
JOIN Gym ON Trainer.hometown=Gym.city
WHERE Gym.leader_id<>Trainer.id
ORDER BY Trainer.name ASC;