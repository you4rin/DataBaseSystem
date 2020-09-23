SELECT Pokemon.type,COUNT(Pokemon.id)
FROM Pokemon
GROUP BY Pokemon.type
ORDER BY COUNT(Pokemon.id) ASC;