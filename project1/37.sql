SELECT x.name, SUM(a.level)
FROM Trainer x
JOIN CatchedPokemon a ON x.id=a.owner_id
GROUP BY x.id
HAVING SUM(a.level)>=ALL(
  SELECT SUM(b.level)
  FROM Trainer y
  JOIN CatchedPokemon b ON y.id=b.owner_id
  GROUP BY y.id
  )
ORDER BY x.name ASC;