SELECT x.name
FROM Evolution a
JOIN Pokemon x ON a.after_id=x.id
WHERE x.id NOT IN(
  SELECT y.id 
  FROM Pokemon y
  JOIN Evolution b ON y.id=b.before_id
  )
AND x.id IN(
  SELECT z.id 
  FROM Pokemon z
  JOIN Evolution c ON z.id=c.after_id
  )
ORDER BY x.name ASC;