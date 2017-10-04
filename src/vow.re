type handled;

type unhandled;

type t 'a 'status = {promise: Js.Promise.t 'a};

let return x => {promise: Js.Promise.resolve x};

let map transform vow => {promise: Js.Promise.then_ (fun x => (transform x).promise) vow.promise};

let mapUnhandled transform vow => {
  promise: Js.Promise.then_ (fun x => (transform x).promise) vow.promise
};

let sideEffect handler vow => {
  let _ = Js.Promise.then_ (fun x => Js.Promise.resolve @@ handler x) vow.promise;
  ()
};

let onError handler vow => {promise: Js.Promise.catch (fun _ => (handler ()).promise) vow.promise};

let wrap promise => {promise: promise};

let unsafeWrap promise => {promise: promise};

let unwrap {promise} => promise;

module type ResultType = {
  type result 'value 'error;
  type vow 'a 'status = t 'a 'status;
  type t 'value 'error 'status = vow (result 'value 'error) 'status;
  let return: 'value => t 'value 'error handled;
  let fail: 'error => t 'value 'error handled;
  let map: ('a => t 'b 'error 'status) => t 'a 'error handled => t 'b 'error 'status;
  let mapUnhandled: ('a => t 'b 'error 'status) => t 'a 'error unhandled => t 'b 'error unhandled;
  let sideEffect: ([ | `Success 'value | `Fail 'error] => unit) => t 'value 'error handled => unit;
  let onError:
    (unit => t 'error 'value 'status) => t 'error 'value unhandled => t 'error 'value 'status;
};

module Result: ResultType = {
  type result 'value 'error = [ | `Success 'value | `Fail 'error];
  type vow 'a 'status = t 'a 'status;
  type t 'value 'error 'status = vow (result 'value 'error) 'status;
  let vowReturn = return;
  let return value => vowReturn (`Success value);
  let fail error => vowReturn (`Fail error);
  let map transform vow =>
    map
      (
        fun x =>
          switch x {
          | `Success x => transform x
          | `Fail x => fail x
          }
      )
      vow;
  let mapUnhandled transform vow =>
    mapUnhandled
      (
        fun x =>
          switch x {
          | `Success x => transform x
          | `Fail x => fail x
          }
      )
      vow;
  let sideEffect handler vow => sideEffect handler vow;
  let onError handler vow => onError handler vow;
};
