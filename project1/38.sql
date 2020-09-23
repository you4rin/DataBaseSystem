SELECT x.name
FROM Pokemon x
WHERE x.id NOT IN(
  SELECT y.id 
  FROM Pokemon y
  JOIN Evolution ON y.id=Evolution.before_id
  )
AND x.id IN(
  SELECT z.id 
  FROM Pokemon z
  JOIN Evolution ON z.id=Evolution.after_id
  )
ORDER BY x.name ASC;