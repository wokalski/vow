module Vow = {
  type handled;
  type unhandled;
  type inner('a, 'status);
  type t('a, 'status) = {promise: Js.Promise.t(inner('a, 'status))};
  /* BEGIN: SECTION OF VERY BAD THINGS */
  /* Taking a fighting fire with fire stategy. Esentially type safety is ironically forcing us to not be able to soundly type promises. Since JS has no such limitation, it can box and unbox intelligently without
     the type system being the wiser. */
  [%%bs.raw
    {|
const innerReturn = (p) => {
  if (Promise.resolve(p) === p) {
    return Promise.resolve({ inner: p });
  } else {
    return Js.Promise.resolve(p);
  }
};

const innerUnwrap = (p) => {
  if (p.inner && Promise.resolve(p.inner) === p.inner) {
    return p.inner;
  } else {
    return p;
  }
};
  |}
  ];
  [@bs.val] external innerReturn : 'a => Js.Promise.t(inner('a, 'status)) = "";
  [@bs.val] external innerUnwrap : inner('a, 'status) => 'a = "";
  external fromPromise : Js.Promise.t('a) => Js.Promise.t(inner('a, 'status)) = "%identity";
  /* END: SECTION OF VERY BAD THINGS */
  let return: 'a => t('a, 'status) = (x) => {promise: innerReturn(x)};
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
  let wrap = (promise) => {promise: fromPromise(promise)};
  let unsafeWrap = (promise) => {promise: fromPromise(promise)};
  let unwrap = ({promise}) => Js.Promise.then_((x) => Js.Promise.resolve(innerUnwrap(x)), promise);
};

module type ResultType = {
  type result('value, 'error) = [ | `Success('value) | `Fail('error)];
  type vow('a, 'status) = Vow.t('a, 'status);
  type t('value, 'error, 'status) = vow(result('value, 'error), 'status);
  let return: 'value => t('value, 'error, Vow.handled);
  let fail: 'error => t('value, 'error, Vow.handled);
  let flatMap:
    ('value1 => t('value2, 'error, 'status), t('value1, 'error, Vow.handled)) =>
    t('b, 'error, 'status);
  let mapError:
    ('a => t('value, 'b, Vow.handled), t('value, 'a, 'status)) => t('value, 'b, 'status);
  let sideEffect:
    ([ | `Success('value) | `Fail('error)] => unit, t('value, 'error, Vow.handled)) => unit;
  let onError:
    (unit => t('error, 'value, 'status), t('error, 'value, Vow.unhandled)) =>
    t('error, 'value, 'status);
  let wrap: (Js.Promise.t('value), unit => 'error) => t('value, 'error, Vow.handled);
  let unwrap:
    ([ | `Success('value) | `Fail('error)] => vow('a, 'status), t('value, 'error, Vow.handled)) =>
    vow('a, 'status);
  module Infix: {};
};

module Result = {
  type result('value, 'error) = [ | `Success('value) | `Fail('error)];
  type vow('a, 'status) = Vow.t('a, 'status);
  type t('value, 'error, 'status) = vow(result('value, 'error), 'status);
  let return = (value) => Vow.return(`Success(value));
  let fail = (error) => Vow.return(`Fail(error));
  let flatMap:
    ('value1 => t('value2, 'error, 'status), t('value1, 'error, Vow.handled)) =>
    t('b, 'error, 'status) =
    (transform, vow) =>
      Vow.flatMap(
        fun
        | `Success(value) => transform(value)
        | `Fail(err) => fail(err),
        vow
      );
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
    /* let (>|=) = (v, t) => map(t, v); */ 36;
    /* let (>>=) = (v, t) => flatMap(t, v); */
  };
};

include Vow;
/* let flatMapUnhandled = (transform, vow) =>
     Vow.flatMapUnhandled(
       (x) =>
         switch x {
         | `Success(x) => transform(x)
         | `Fail(x) => fail(x)
         },
       vow
     );
   /* let map = (transform, vow) => flatMap((x) => return(transform(x)), vow); */
   let mapUnhandled = (transform, vow) => flatMapUnhandled((x) => return(transform(x)), vow); */