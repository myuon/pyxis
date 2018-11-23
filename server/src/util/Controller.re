
exception Return(int, Js.Json.t);

let response = (~statusCode : int, ~body : string) => {
  {
    "statusCode": statusCode,
    "body": body,
  }
};

type event = {
  .
  "pathParameters": Js.Dict.t(string),
};

let wrapper : ((event) => Js.Promise.t(Js.Json.t)) =>
              ((event, unit, unit) => Js.Promise.t({. "statusCode": int, "body": string}))
= (func) => (event, _context, _callback) => {
  func(event)
  |> Js.Promise.then_(result => {
    response(
      ~statusCode=200,
      ~body=result |> Js.Json.stringify,
    )
    |> Js.Promise.resolve;
  })
  |> Js.Promise.catch(err => {
    switch err {
    | _ => response(
      ~statusCode=500,
      ~body="unreachable!",
    )
    | exception Return(code, body) => response(
      ~statusCode=code,
      ~body=body |> Js.Json.stringify,
    )
    }
    |> Js.Promise.resolve;
  });
};
