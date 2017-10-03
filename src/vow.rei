type handled;

type unhandled;

type vow 'a 'status;

let sideEffect: ('a => unit) => vow 'a handled => unit;

let onSuccessResolve: ('a => 'b) => vow 'a handled => vow 'b handled;

let andThen: ('a => vow 'b 'status) => vow 'a handled => vow 'b 'status;

let onErrorHandle: (unit => vow 'a 'status) => vow 'a unhandled => vow 'a 'status;

let onError: 'a => vow 'a unhandled => vow 'a handled;

let wrap: Js.Promise.t 'a => vow 'a unhandled;

let unsafeWrap: Js.Promise.t 'a => vow 'a handled;

let unwrap: vow 'a handled => Js.Promise.t 'a;
