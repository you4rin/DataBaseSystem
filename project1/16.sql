SELECT COUNT(Pokemon.id)
FROM Pokemon
WHERE Pokemon.type='Water'
OR Pokemon.type='Electric'
OR Pokemon.type='Psychic';