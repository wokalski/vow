type handled;

type unhandled;

type vow 'a 'status = Js.Promise.t 'a;

let sideEffect handler vow => {
  let _ = Js.Promise.then_ (fun x => Js.Promise.resolve @@ handler x) vow;
  ()
};

let onSuccessResolve transform vow =>
  Js.Promise.then_ (fun x => Js.Promise.resolve @@ transform x) vow;

let andThen try_ vow =>
  Js.Promise.then_ try_ vow;


let onErrorHandle handle vow =>
  Js.Promise.catch (fun _ => handle ()) vow;

let onError value vow =>
  onErrorHandle (fun _ => Js.Promise.resolve value) vow;

let wrap promise => promise;

let unsafeWrap promise => promise;

let unwrap promise => promise;
