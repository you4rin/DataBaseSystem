SELECT Pokemon.name
FROM Pokemon
JOIN CatchedPokemon ON CatchedPokemon.pid=Pokemon.id
WHERE CatchedPokemon.nickname LIKE '% %'
ORDER BY Pokemon.name DESC;