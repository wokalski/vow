# Vow

`Vow` is a tiny library which allows you to handle promises more safely in your Bucklescript application.

A `Vow` can be either `handled` and `unhandled`. All promises of type `vow 'a handled` make sure that you handled Promise rejections. Thanks to that you will avoid the Uncaught promise error.

## Side effects

After series of operations you usually want to "consume" a promise. `Vow.sideEffect` should be used for that.

It only accepts promises which are properly handled.

## Unwrapping

You can unwrap a handled promise using `Vow.unwrap`.

## Nesting vows

If you nest `vow`s (which are really just `Js.Promise.t` under the scenes) you're gonna get a `vow` of the following type:

```reason
/* in Reason syntax */

vow (vow 'a 'status1) 'status2
```
However, under the scenes it'll really be

```reason

vow 'a 'status
```

Therefore `vow` is not sound.

## Binding

In order to use vows you have to bind to your existing APIs using `Vow.wrap`/`Vow.unsafeWrap`.

If you `unsafeWrap` a promise which does throw your code will be unsound.


