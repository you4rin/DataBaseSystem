SELECT DISTINCT Pokemon.name, Pokemon.type
FROM Pokemon
JOIN CatchedPokemon ON CatchedPokemon.pid=Pokemon.id
WHERE CatchedPokemon.level>=30
ORDER BY Pokemon.name ASC;