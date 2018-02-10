module Vow = {
  type handled;
  type unhandled;
  type container('a) = {value: 'a};
  type t('a, 'status) = {promise: Js.Promise.t(container('a))};
  external toHandled : t('a, 'status1) => t('a, handled) = "%identity";
  let innerReturn = (v) => {value: v};
  let outerWrap: Js.Promise.t(container('a)) => t('a, 'status) = (promise) => {promise: promise};
  let return = (x) => outerWrap(Js.Promise.resolve(innerReturn(x)));
  let flatMap = (transform, vow) =>
    outerWrap(Js.Promise.then_((x) => transform(x.value).promise, vow.promise));
  let flatMapUnhandled = (transform, vow) =>
    outerWrap(Js.Promise.then_((inn) => transform(inn.value).promise, vow.promise));
  let map = (transform, vow) => flatMap((x) => return(transform(x)), vow);
  let mapUnhandled = (transform, vow) => flatMapUnhandled((x) => return(transform(x)), vow);
  let sideEffect = (handler, vow) => {
    let _ = Js.Promise.then_((x) => Js.Promise.resolve(handler(x.value)), vow.promise);
    ()
  };
  let onError = (handler, vow) =>
    outerWrap(Js.Promise.catch((_) => handler().promise, vow.promise));
  let wrap = (promise) =>
    outerWrap(Js.Promise.then_((res) => Js.Promise.resolve(innerReturn(res)), promise));
  let unsafeWrap = (promise) =>
    outerWrap(Js.Promise.then_((res) => Js.Promise.resolve(innerReturn(res)), promise));
  let unwrap = (vow) => vow.promise;
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
  type t('value, 'error, 'status) = Vow.t(result('value, 'error), 'status);
  let return = (value) => Vow.return(`Success(value));
  let fail = (error) => Vow.return(`Fail(error));
  let flatMap = (transform, vow) =>
    Vow.flatMap(
      (x) =>
        switch x {
        | `Success(x) => transform(x)
        | `Fail(x) => fail(x)
        },
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
        | `Success(x) => return(x) |> Vow.toHandled
        | `Fail(x) => transform(x) |> Vow.toHandled
        },
      vow
    );
  let sideEffect = (handler, vow) => Vow.sideEffect(handler, vow);
  let onError = (handler, vow) => Vow.onError(handler, vow);
  let wrap = (promise, handler) =>
    Vow.wrap(promise) |> Vow.flatMapUnhandled(return) |> onError(() => fail(handler()));
  let unwrap = (transform, vow) => Vow.flatMap(transform, vow);
  module Infix = {
    let (>>=) = (v, t) => flatMap(t, v);
    let (>|=) = (v, t) => map(t, v);
  };
};

include Vow;