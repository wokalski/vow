Vow.return("hello") |> Vow.sideEffect(Js.log);

/* Vow inside vow */
Vow.return(Vow.return("hello")) |> Vow.sideEffect((x) => x |> Vow.sideEffect(Js.log));

/* Promise inside vow */
Vow.return(Js.Promise.resolve("hello"))
|> Vow.sideEffect((r) => Js.Promise.then_((str) => Js.Promise.resolve(Js.log(str)), r) |> ignore);

Vow.return(Js.Promise.resolve("hello"))
|> Vow.map(Js.Promise.then_((str) => Js.Promise.resolve(str ++ " world")))
|> Vow.map((r) => Js.Promise.then_((str) => Js.Promise.resolve(Js.log(str)), r));

/* Vow inside promise */
Js.Promise.resolve(Vow.return("hello"))
|> Js.Promise.then_((strVow) => Js.Promise.resolve(Vow.sideEffect(Js.log, strVow)));

Vow.return(Js.Promise.resolve("unwrapping"))
|> Vow.unwrap
|> Js.Promise.then_(
     (promise: Vow.container(_)) =>
       Js.Promise.then_((x) => Js.log(x) |> Js.Promise.resolve, promise.value)
   )
|> ignore;