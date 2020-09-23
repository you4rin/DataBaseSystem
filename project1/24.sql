SELECT City.name, AVG(CatchedPokemon.level) AS level
FROM Trainer
RIGHT OUTER JOIN City ON City.name=Trainer.hometown
LEFT OUTER JOIN CatchedPokemon ON Trainer.id=CatchedPokemon.owner_id
GROUP BY City.name
ORDER BY AVG(CatchedPokemon.level) ASC;