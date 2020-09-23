SELECT x.name, AVG(a.level)
FROM Trainer x
JOIN CatchedPokemon a ON x.id=a.owner_id
JOIN Pokemon ON a.pid=Pokemon.id
WHERE Pokemon.type='Electric'
OR Pokemon.type='Normal'
GROUP BY x.id
ORDER BY AVG(a.level) ASC;