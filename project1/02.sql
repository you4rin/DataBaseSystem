SELECT name
FROM(
  SELECT x.name
  FROM Pokemon x
  WHERE x.type IN(
    SELECT y.type
    FROM Pokemon y
    GROUP BY y.type
    HAVING COUNT(y.id)>=ALL(
      SELECT COUNT(z.id)
      FROM Pokemon z
      WHERE z.type NOT IN(
        SELECT w.type
        FROM Pokemon w
        GROUP BY w.type
        HAVING COUNT(w.id)>=ALL(
          SELECT COUNT(v.id)
          FROM Pokemon v
          GROUP BY v.type
          )
        )
      GROUP BY z.type
      )
    )
)A
ORDER BY name;