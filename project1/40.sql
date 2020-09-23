SELECT City.name, a.nickname
FROM City
LEFT OUTER JOIN Trainer x ON x.hometown=City.name
LEFT OUTER JOIN CatchedPokemon a ON x.id=a.owner_id
WHERE a.level>=ALL(
  SELECT b.level
  FROM Trainer y
  JOIN CatchedPokemon b ON y.id=b.owner_id
  WHERE x.hometown=y.hometown
  )
ORDER BY City.name ASC;
