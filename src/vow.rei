type handled;

type unhandled;

type t 'a 'status;


/**
 * Returns a value wrapped in a vow
 */
let return: 'a => t 'a handled;


/**
 * Maps a handled vow with value of type 'a to a vow returned by the transform function
 */
let flatMap: ('a => t 'b 'status) => t 'a handled => t 'b 'status;


/**
 * Maps an unhandled vow with value of type 'a to a vow returned by the transform function.
 * The returned vow is unhandled.
 */
let flatMapUnhandled: ('a => t 'b 'status) => t 'a unhandled => t 'b unhandled;


/**
 * Maps a handled vow with value of type 'a to a vow of the value
 * returned by the transform function
 */
let map: ('a => 'b) => t 'a handled => t 'b 'status;


/**
 * Maps a handled vow with value of type 'a to a vow of the value
 * returned by the transform function
 */
let mapUnhandled: ('a => 'b) => t 'a unhandled => t 'b unhandled;


/**
 * Performs side effects with a handled vow's value and returns unit.
 */
let sideEffect: ('a => unit) => t 'a handled => unit;


/**
 * Catches and handles the rejection of a backing promise.
 */
let onError: (unit => t 'a 'status) => t 'a unhandled => t 'a 'status;


/**
 * Wraps a promise into a vow. You should use this function for wrapping promises that
 * might be rejected
 */
let wrap: Js.Promise.t 'a => t 'a unhandled;


/**
 * Wraps a non failing promise into a vow. Use this function if your the wrapped promise
 * is never rejected.
 */
let unsafeWrap: Js.Promise.t 'a => t 'a handled;


/**
 * Returns the underlying JS Promise.
 */
let unwrap: t 'a handled => Js.Promise.t 'a;

module type ResultType = {
  type result 'value 'error;
  type vow 'a 'status = t 'a 'status;
  type t 'value 'error 'status = vow (result 'value 'error) 'status;
  let return: 'value => t 'value 'error handled;
  let fail: 'error => t 'value 'error handled;
  let flatMap: ('a => t 'b 'error 'status) => t 'a 'error handled => t 'b 'error 'status;
  let flatMapUnhandled: ('a => t 'b 'error 'status) => t 'a 'error unhandled => t 'b 'error unhandled;
  let map: ('a => 'b) => t 'a 'error handled => t 'b 'error 'status;
  let mapUnhandled: ('a => 'b) => t 'a 'error unhandled => t 'b 'error unhandled;
  let mapError: ('a => t 'value 'b handled) => t 'value 'a 'status => t 'value 'b 'status;
  let sideEffect: ([ | `Success 'value | `Fail 'error] => unit) => t 'value 'error handled => unit;
  let onError:
    (unit => t 'error 'value 'status) => t 'error 'value unhandled => t 'error 'value 'status;
  let wrap: Js.Promise.t 'value => (unit => 'error) => t 'value 'error handled;
  let unwrap:
    ([ | `Success 'value | `Fail 'error] => vow 'a 'status) =>
    t 'value 'error handled =>
    vow 'a 'status;
  module Infix: {
    let (>>=): t 'a 'error handled => ('a => t 'b 'error 'status) => t 'b 'error 'status';
    let (>|=): t 'a 'error handled => ('a => 'b) => t 'b 'error handled;
  };
};

module Result: ResultType;