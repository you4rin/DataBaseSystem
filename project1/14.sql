SELECT Pokemon.name
FROM Pokemon
JOIN Evolution ON Pokemon.id=Evolution.before_id
WHERE Pokemon.type='Grass'