SELECT x.id, x.name, y.name, z.name
FROM Pokemon x
JOIN Evolution a ON x.id=a.before_id
JOIN Evolution b ON b.before_id=a.after_id
JOIN Pokemon y ON a.after_id=y.id
JOIN Pokemon z ON b.after_id=z.id
WHERE x.id NOT IN(
  SELECT c.after_id
  FROM Evolution c
  )
AND z.id NOT IN(
  SELECT d.before_id
  FROM Evolution d
  )
ORDER BY x.id ASC;