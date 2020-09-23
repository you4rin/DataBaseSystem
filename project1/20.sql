SELECT Trainer.name,COUNT(CatchedPokemon.id)
FROM Trainer
JOIN CatchedPokemon ON Trainer.id=CatchedPokemon.owner_id
WHERE Trainer.hometown='Sangnok City'
GROUP BY Trainer.name
ORDER BY COUNT(CatchedPokemon.id) ASC;