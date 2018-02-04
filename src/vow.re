module Vow = {
  type handled;
  type unhandled;
  type inner('a) = {inner: 'a};
  type t('a, 'status) = {promise: Js.Promise.t(inner('a))};
  /* BEGIN: SECTION OF VERY BAD THINGS */
  let innerReturn = (v) => {inner: v};
  let innerUnwrap = (v) => v.inner;
  /* END: SECTION OF VERY BAD THINGS */
  let return: 'a => t('a, 'status) = (x) => {promise: Js.Promise.resolve(innerReturn(x))};
  let flatMap = (transform, vow) => {
    promise: Js.Promise.then_((x) => transform(innerUnwrap(x)).promise, vow.promise)
  };
  let flatMapUnhandled: ('a => t('b, 'status), t('a, unhandled)) => t('b, unhandled) =
    (transform, vow) => {
      promise: Js.Promise.then_((x) => transform(innerUnwrap(x)).promise, vow.promise)
    };
  let map = (transform, vow) => flatMap((x) => return(transform(x)), vow);
  let mapUnhandled: ('a => 'b, t('a, unhandled)) => t('b, unhandled) =
    (transform, vow) => flatMapUnhandled((x) => return(transform(x)), vow);
  let sideEffect = (handler, vow) => {
    let _ = Js.Promise.then_((x) => Js.Promise.resolve @@ handler(innerUnwrap(x)), vow.promise);
    ()
  };
  let onError = (handler, vow) => {
    promise: Js.Promise.catch((_) => handler().promise, vow.promise)
  };
  let wrap = (promise) => {
    promise: Js.Promise.then_((res) => Js.Promise.resolve(innerReturn(res)), promise)
  };
  let unsafeWrap = (promise) => {
    promise: Js.Promise.then_((res) => Js.Promise.resolve(innerReturn(res)), promise)
  };
  let unwrap = ({promise}) => Js.Promise.then_((x) => Js.Promise.resolve(innerUnwrap(x)), promise);
};

module type ResultType = {
  open Vow;
  type result('value, 'error);
  type vow('a, 'status) = t('a, 'status);
  type t('value, 'error, 'status) = vow(result('value, 'error), 'status);
  let return: 'value => t('value, 'error, handled);
  let fail: 'error => t('value, 'error, handled);
  let flatMap: ('a => t('b, 'error, 'status), t('a, 'error, handled)) => t('b, 'error, 'status);
  let flatMapUnhandled:
    ('a => t('b, 'error, 'status), t('a, 'error, unhandled)) => t('b, 'error, unhandled);
  let map: ('a => 'b, t('a, 'error, handled)) => t('b, 'error, 'status);
  let mapUnhandled: ('a => 'b, t('a, 'error, unhandled)) => t('b, 'error, unhandled);
  let mapError: ('a => t('value, 'b, handled), t('value, 'a, 'status)) => t('value, 'b, 'status);
  let sideEffect:
    ([ | `Success('value) | `Fail('error)] => unit, t('value, 'error, handled)) => unit;
  let onError:
    (unit => t('error, 'value, 'status), t('error, 'value, unhandled)) =>
    t('error, 'value, 'status);
  let wrap: (Js.Promise.t('value), unit => 'error) => t('value, 'error, handled);
  let unwrap:
    ([ | `Success('value) | `Fail('error)] => vow('a, 'status), t('value, 'error, handled)) =>
    vow('a, 'status);
  module Infix: {
    let (>>=): (t('a, 'error, handled), 'a => t('b, 'error, 'status)) => t('b, 'error, 'status');
    let (>|=): (t('a, 'error, handled), 'a => 'b) => t('b, 'error, handled);
  };
};

module Result = {
  type result('value, 'error) = [ | `Success('value) | `Fail('error)];
  type vow('a, 'status) = Vow.t('a, 'status);
  type t('value, 'error, 'status) = vow(result('value, 'error), 'status);
  let return = (value) => Vow.return(`Success(value));
  let fail = (error) => Vow.return(`Fail(error));
  let flatMap = (transform, vow) =>
    Vow.flatMap(
      fun
      | `Success(value) => transform(value)
      | `Fail(err) => fail(err),
      vow
    );
  let flatMapUnhandled = (transform, vow) =>
    Vow.flatMapUnhandled(
      (x) =>
        switch x {
        | `Success(x) => transform(x)
        | `Fail(x) => fail(x)
        },
      vow
    );
  let map = (transform, vow) => flatMap((x) => return(transform(x)), vow);
  let mapUnhandled = (transform, vow) => flatMapUnhandled((x) => return(transform(x)), vow);
  let mapError = (transform, vow) =>
    Vow.flatMap(
      (x) =>
        switch x {
        | `Success(x) => return(x)
        | `Fail(x) => transform(x)
        },
      vow
    );
  let sideEffect = (handler, vow) => Vow.sideEffect(handler, vow);
  let onError = (handler, vow) => Vow.onError(handler, vow);
  let wrap = (promise, handler) =>
    Vow.wrap(promise) |> Vow.flatMapUnhandled((x) => return(x)) |> onError(() => fail(handler()));
  let unwrap = (transform, vow) => Vow.flatMap(transform, vow);
  module Infix = {
    let (>>=):
      (t('a, 'error, Vow.handled), 'a => t('b, 'error, 'status)) => t('b, 'error, 'status') =
      (v, t) => flatMap(t, v);
    let (>|=): (t('a, 'error, Vow.handled), 'a => 'b) => t('b, 'error, Vow.handled) =
      (v, t) => map(t, v);
  };
};

include Vow;