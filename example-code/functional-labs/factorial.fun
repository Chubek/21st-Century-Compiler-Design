(letrec fact
  (lam n (if (eq n 0) 1 (mul n (app fact (add n -1)))))
  (app fact 6))
