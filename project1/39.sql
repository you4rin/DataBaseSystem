SELECT name
FROM(
  SELECT DISTINCT x.name, a.pid
  FROM Trainer x
  JOIN CatchedPokemon a ON a.owner_id=x.id
  JOIN CatchedPokemon b ON b.owner_id=x.id
  WHERE a.pid=b.pid
  AND a.id<>b.id
  ORDER BY x.name
)CNT;