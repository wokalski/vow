module Vow = {
  type handled;
  type unhandled;
  type container('a) = {value: 'a};
  type t('a, 'status) = {promise: Js.Promise.t(container('a))};
  let innerReturn = v => {value: v};
  let return = x => {promise: Js.Promise.resolve(innerReturn(x))};
  let flatMap = (transform, vow) => {
    promise: Js.Promise.then_(x => transform(x.value).promise, vow.promise),
  };
  let flatMapUnhandled = (transform, vow) => {
    promise:
      Js.Promise.then_(inn => transform(inn.value).promise, vow.promise),
  };
  let map = (transform, vow) => flatMap(x => return(transform(x)), vow);
  let mapUnhandled = (transform, vow) =>
    flatMapUnhandled(x => return(transform(x)), vow);
  let sideEffect = (handler, vow) => {
    let _ =
      Js.Promise.then_(
        x => Js.Promise.resolve(handler(x.value)),
        vow.promise,
      );
    ();
  };
  let onError = (handler, vow) => {
    promise: Js.Promise.catch(_ => handler().promise, vow.promise),
  };
  let wrap = promise => {
    promise:
      Js.Promise.then_(
        res => Js.Promise.resolve(innerReturn(res)),
        promise,
      ),
  };
  let unsafeWrap = promise => {
    promise:
      Js.Promise.then_(
        res => Js.Promise.resolve(innerReturn(res)),
        promise,
      ),
  };
  let unwrap = ({promise}) =>
    promise
    |> Js.Promise.(then_(({value}: container('a)) => value |> resolve));
  let all2 = ((v1, v2)) =>
    v1
    |> flatMap(v1Result =>
         v2 |> flatMap(v2Result => return((v1Result, v2Result)))
       );
  let all3 = ((v1, v2, v3)) =>
    all2((v1, v2))
    |> flatMap(((v1Result, v2Result)) =>
         v3 |> flatMap(v3Result => return((v1Result, v2Result, v3Result)))
       );
  let all4 = ((v1, v2, v3, v4)) =>
    all3((v1, v2, v3))
    |> flatMap(((v1Result, v2Result, v3Result)) =>
         v4
         |> flatMap(v4Result =>
              return((v1Result, v2Result, v3Result, v4Result))
            )
       );
  let rec all = vows =>
    switch (vows) {
    | [head, ...tail] =>
      all(tail)
      |> flatMap(tailResult =>
           head |> flatMap(headResult => return([headResult, ...tailResult]))
         )
    | [] => return([])
    };
};

module type ResultType = {
  open Vow;
  type vow('a, 'status) = t('a, 'status);
  type t('value, 'error, 'status) =
    vow(Belt.Result.t('value, 'error), 'status);
  let return: 'value => t('value, 'error, handled);
  let fail: 'error => t('value, 'error, handled);
  let flatMap:
    ('a => t('b, 'error, 'status), t('a, 'error, handled)) =>
    t('b, 'error, 'status);
  let flatMapUnhandled:
    ('a => t('b, 'error, 'status), t('a, 'error, unhandled)) =>
    t('b, 'error, unhandled);
  let map: ('a => 'b, t('a, 'error, handled)) => t('b, 'error, 'status);
  let mapUnhandled:
    ('a => 'b, t('a, 'error, unhandled)) => t('b, 'error, unhandled);
  let mapError:
    ('a => t('value, 'b, handled), t('value, 'a, 'status)) =>
    t('value, 'b, 'status);
  let sideEffect:
    (Belt.Result.t('value, 'error) => unit, t('value, 'error, handled)) =>
    unit;
  let onError:
    (unit => t('error, 'value, 'status), t('error, 'value, unhandled)) =>
    t('error, 'value, 'status);
  let wrap:
    (Js.Promise.t('value), unit => 'error) => t('value, 'error, handled);
  let unwrap:
    (
      Belt.Result.t('value, 'error) => vow('a, 'status),
      t('value, 'error, handled)
    ) =>
    vow('a, 'status);
  let all2:
    ((t('v1, 'error, handled), t('v2, 'error, handled))) =>
    t(('v1, 'v2), 'error, handled);
  let all3:
    (
      (
        t('v1, 'error, handled),
        t('v2, 'error, handled),
        t('v3, 'error, handled),
      )
    ) =>
    t(('v1, 'v2, 'v3), 'error, handled);
  let all4:
    (
      (
        t('v1, 'error, handled),
        t('v2, 'error, handled),
        t('v3, 'error, handled),
        t('v4, 'error, handled),
      )
    ) =>
    t(('v1, 'v2, 'v3, 'v4), 'error, handled);
  let all:
    list(t('value, 'error, handled)) => t(list('value), 'error, handled);
  module Infix: {
    let (>>=):
      (t('a, 'error, handled), 'a => t('b, 'error, 'status)) =>
      t('b, 'error, 'status');
    let (>|=): (t('a, 'error, handled), 'a => 'b) => t('b, 'error, handled);
  };
};

module Result: ResultType = {
  type vow('a, 'status) = Vow.t('a, 'status);
  type t('value, 'error, 'status) =
    vow(Belt.Result.t('value, 'error), 'status);
  let return = value => Vow.return(Belt.Result.Ok(value));
  let fail = error => Vow.return(Belt.Result.Error(error));
  let flatMap = (transform, vow) =>
    Vow.flatMap(
      x =>
        switch (x) {
        | Belt.Result.Ok(x) => transform(x)
        | Belt.Result.Error(x) => fail(x)
        },
      vow,
    );
  let flatMapUnhandled = (transform, vow) =>
    Vow.flatMapUnhandled(
      x =>
        switch (x) {
        | Belt.Result.Ok(x) => transform(x)
        | Belt.Result.Error(x) => fail(x)
        },
      vow,
    );
  let map = (transform, vow) => flatMap(x => return(transform(x)), vow);
  let mapUnhandled = (transform, vow) =>
    flatMapUnhandled(x => return(transform(x)), vow);
  let mapError = (transform, vow) =>
    Vow.flatMap(
      x =>
        switch (x) {
        | Belt.Result.Ok(x) => return(x)
        | Belt.Result.Error(x) => transform(x)
        },
      vow,
    );
  let sideEffect = (handler, vow) => Vow.sideEffect(handler, vow);
  let onError = (handler, vow) => Vow.onError(handler, vow);
  let wrap = (promise, handler) =>
    Vow.wrap(promise)
    |> Vow.flatMapUnhandled(x => return(x))
    |> onError(() => fail(handler()));
  let unwrap = (transform, vow) => Vow.flatMap(transform, vow);
  let all2 = ((v1, v2)) =>
    v1
    |> flatMap(v1Result =>
         v2 |> flatMap(v2Result => return((v1Result, v2Result)))
       );
  let all3 = ((v1, v2, v3)) =>
    all2((v1, v2))
    |> flatMap(((v1Result, v2Result)) =>
         v3 |> flatMap(v3Result => return((v1Result, v2Result, v3Result)))
       );
  let all4 = ((v1, v2, v3, v4)) =>
    all3((v1, v2, v3))
    |> flatMap(((v1Result, v2Result, v3Result)) =>
         v4
         |> flatMap(v4Result =>
              return((v1Result, v2Result, v3Result, v4Result))
            )
       );
  let rec all = vows =>
    switch (vows) {
    | [head, ...tail] =>
      all(tail)
      |> flatMap(tailResult =>
           head |> flatMap(headResult => return([headResult, ...tailResult]))
         )
    | [] => return([])
    };
  module Infix = {
    let (>>=) = (v, t) => flatMap(t, v);
    let (>|=) = (v, t) => map(t, v);
  };
};

include Vow;
