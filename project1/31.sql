SELECT x.type
FROM Pokemon x
JOIN Evolution a ON x.id=a.before_id
GROUP BY x.type
HAVING COUNT(x.id)>=3
ORDER BY x.type DESC;