SELECT COUNT(CatchedPokemon.id)
FROM CatchedPokemon
JOIN Pokemon ON Pokemon.id=CatchedPokemon.pid
GROUP BY Pokemon.type
ORDER BY Pokemon.type;