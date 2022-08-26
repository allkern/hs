<p align="center">
  <img width="25%" height="25%" src="https://user-images.githubusercontent.com/15825466/186776504-8557ab50-6487-4387-a874-16a674b85c61.png" alt="HS">
</p>

# hs
A programming language where *everything is an expression*

Ever wanted to *add 2* to a *function* and then store it in a variable? Ever wanted to call a number?

Now you can!

```haskell
fn main -> int: {
  int a = (fn: print("Hello, world!")) + 2;

  0xdeadbeef(a, 4);
};
```

Why? Because *why not*? It's my language, and **I** make the rules.

<p align="center">
  <img width="50%" height="50%" src="https://user-images.githubusercontent.com/15825466/186779993-7f9f7c0d-7fb4-46c3-9782-c9068263909a.png" alt="HS">
</p>


## Ok, but, for real
All of these (reluctantly valid) "features" are just a side-effect of hs' flexibility. Which comes from the fact that everything on hs is an expression. And as such, you can do whatever you want, with whatever you define.

But, what does hs say an "expression" is?

For hs, an expression is *something* that can either be compiled into code, be evaluated into a value, or both. In this sense, a function is an expression that emits code for its body, handling arguments, and returning values. But which also returns a pointer to the aforementioned function! So, you could just (figuratively) add 2 to a function you just defined. Or use any of hs' operators on it.

You could access a function's binary code using hs' array access expression:

```haskell
(fn dummy: { int a = 10; a += 20; a; })[0x1]
```

You could save an anonymous function into a variable, then instantly call it:

```haskell
(u32 my_lambda = (fn (i: int) -> char: 'a' + i))(10)
```

And much, much more. As you can see, we can condense fairly complex concepts into powerful, yet straightforward snippets, and this is just the beginning!

I'll eventually make a nicer README, or a wiki for you to discover more of hs' expressions and operators, for now, this is all I can do. Stay tuned!
