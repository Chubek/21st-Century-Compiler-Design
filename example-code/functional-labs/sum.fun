(letrec sum
  (lam xs (case xs 0 (head tail (add head (app sum tail)))))
  (app sum (cons 4 (cons 7 (cons 9 nil)))))
