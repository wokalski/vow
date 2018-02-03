open Vow;

Vow.return(Js.Promise.resolve("hello"))
|> Vow.sideEffect((r) => Js.Promise.then_((str) => Js.Promise.resolve(Js.log(str)), r) |> ignore);