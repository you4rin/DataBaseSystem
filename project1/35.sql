SELECT Pokemon.name
FROM Pokemon
JOIN Evolution ON Evolution.before_id=Pokemon.id
WHERE Evolution.before_id>Evolution.after_id
ORDER BY Pokemon.name ASC;