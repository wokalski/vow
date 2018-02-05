# Vow

`Vow` is a tiny library which allows you to handle promises more safely in your Bucklescript application.

A `Vow` can be either `handled` and `unhandled`. All promises of type `vow 'a handled` make sure that you handled Promise rejections. Thanks to that you will avoid the Uncaught promise error.

## Installation

```sh
npm install --save @wokalski/vow
```

Then add `vow` to `bs-dependencies` in your `bsconfig.json`:
```js
{
  ...
  "bs-dependencies": ["@wokalski/vow"]
}
```

## Side effects

After series of operations you usually want to "consume" a promise. `Vow.sideEffect` should be used for that.

It only accepts promises which are properly handled.

## Unwrapping

You can unwrap a handled promise using `Vow.unwrap`.

## Nesting vows

`Js.Promise.t` is unsafe when you nest promises. i.e. `Js.Promise.t (Js.Promise.t 'a)` is unsound. In the runtime it's `Js.Promise.t`.

This is resolved with `vow`s. If you nest `vow`s they behave as expected.

However if you put a `Js.Promise.t` inside a `vow` (which are boxed `Js.Promise.t` under the scenes) you're gonna get a `vow` of the following type:

```reason
/* in Reason syntax */

vow (Js.Promise.t 'a) 'status
```
However, under the scenes it'll really be

```reason

vow 'a 'status
```

Therefore `vow` is not sound.

## Binding

In order to use vows you have to bind to your existing APIs using `Vow.wrap`/`Vow.unsafeWrap`.

If you `unsafeWrap` a promise which does throw your code will be unsound.

## Example

Let's see a real world example of vows with some comments:

```reason
let login _: Vow.Result.t authenticationState error Vow.handled =>
  /* Returns a handled Vow.Result.t */
  Login.logIn () |>
  /* Validates the returned value. Since the vow is handled we don't need to catch*/
  Vow.Result.map (
    fun x =>
      if x##isCancelled {
        Vow.Result.fail LoginRequestCancelled
      } else {
        Vow.Result.return ()
      }
  ) |>
  /* Another handled Vow.Result.t */
  Vow.Result.map Login.getCurrentAccessToken () |>
  Vow.Result.map (
    fun x => {
      let token = x##accessToken;
      /* This returns an unhandled Vow.Result.t.
       * Note that the 'error types have to match
       * Because after one error the subsequent operations
       * Are not performed.
       */
      Queries.login ::token
    }
  ) |>
  /* Ooops, the `Queries.login` might reject.
   * We are forced to handle it in the compile time.
   */
  Vow.Result.onError (fun _ => Vow.Result.fail GraphQlSignInError) |>
  Vow.Result.map (
    fun x =>
      switch x {
      | Authenticated {token, userId} =>
        /* The promise we wrap is never rejected */
        Vow.unsafeWrap
          KeyChain.(
            Js.Promise.all2 (
              setGenericPassword username::"userId" password::userId service::"userId",
              setGenericPassword username::"token" password::token service::"token"
            )
          ) |>
        Vow.map (fun _ => Vow.Result.return x)
      | _ => Vow.Result.return x
      }
  );
```

## Author

[@wokalski](http://twitter.com/wokalski) 

