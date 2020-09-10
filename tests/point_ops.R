library(s2)
# do some point math operations:
p = s2_point(1,2,3)
p + p
c(p,p)
c(p,p) - p
rep(p, 3)
p * 2
p / 2
d = c(s2_point(1,2,3), s2_point(2,2,2), s2_point(1,1,1))
diff(d)
try(p ^ 2)
