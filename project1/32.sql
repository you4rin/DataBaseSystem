SELECT x.name
FROM Pokemon x
WHERE x.name NOT IN(
  SELECT DISTINCT y.name
  FROM Pokemon y
  JOIN CatchedPokemon ON CatchedPokemon.pid=y.id
  )
ORDER BY x.name ASC;